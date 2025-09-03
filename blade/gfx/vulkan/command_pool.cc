#include "gfx/vulkan/command.h"
#include <climits>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            command_pool::command_pool(
                VkCommandPool pool
                , std::weak_ptr<class device> device
                , const VkAllocationCallbacks* callbacks
            ) noexcept
                : _command_pool{ pool }
                , _device{ device }
                , _allocation_callbacks{ callbacks }
            {}

            std::optional<std::shared_ptr<command_pool>> command_pool::builder::build() const noexcept
            {
                const auto flags = [this]() -> VkCommandPoolCreateFlagBits {
                    switch (info.kind)
                    {
                        case kind::reset:
                            return VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                        case kind::transient:
                            return VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
                    }
                }();

                VkCommandPoolCreateInfo create_info {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                    .flags = flags
                };

                VkCommandPool pool {};
                const VkResult result = vkCreateCommandPool(info.device.lock()->handle(), &create_info, info.allocation_callbacks, &pool);
                
                if (result != VK_SUCCESS)
                {
                    return std::nullopt;
                }

                return std::make_shared<command_pool>(pool, info.device, info.allocation_callbacks);
            }

            command_pool::builder& command_pool::builder::set_queue_family_index(const u32 index) noexcept
            {
                info.queue_family_index = index;

                return *this;
            }
            
            command_pool::builder& command_pool::builder::use_allocation_callbacks(const VkAllocationCallbacks* callbacks) noexcept
            {
                info.allocation_callbacks = callbacks;

                return *this;
            }

            std::optional<command_buffer> command_pool::allocate_single() noexcept
            {
                VkCommandBufferAllocateInfo alloc_info {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    .commandPool = _command_pool,
                    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    .commandBufferCount = 1 // only allocate 1 
                };

                VkCommandBuffer buffer {};
                const VkResult result = vkAllocateCommandBuffers(_device.lock()->handle(), &alloc_info, &buffer);
                if (result != VK_SUCCESS)
                {
                    return std::nullopt;
                }
                
                return command_buffer{ buffer };
            }

            VkCommandBuffer command_pool::acquire_command_buffer() noexcept
            {
                // logger::info("Acquiring buffer");
                
                // Check if free list is empty
                // Reallocate and try again
                if (_free_list_head == nullptr)
                {
                    // logger::info("No buffers available. Reallocating");
                    return VK_NULL_HANDLE;
                    // allocate_buffers(4);
                }

                buffer_node* node = _free_list_head;
                _free_list_head = _free_list_head->next;

                // logger::info("Resetting buffer");
                const u32 flags = 0;
                vkResetCommandBuffer(node->command_buffer, flags);

                node->is_submitted = false;
                _active_buffers[node->command_buffer] = node;
                node->usage_count++;

                return node->command_buffer;
            }

            VkResult command_pool::submit_buffer(
                VkCommandBuffer buffer
                , VkQueue queue
                , const VkSemaphore* wait_semaphores
                , u32 wait_semaphore_count
                , const VkSemaphore* signal_semaphores
                , u32 signal_semaphore_count
                , VkPipelineStageFlags wait_stage
            ) const noexcept {
                auto buffer_it = _active_buffers.find(buffer);
                if (buffer_it == _active_buffers.end())
                {
                    return VK_ERROR_UNKNOWN;
                }

                buffer_node* node = buffer_it->second;

                VkSubmitInfo submit_info {
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .waitSemaphoreCount = wait_semaphore_count,
                    .pWaitSemaphores = wait_semaphores,
                    .pWaitDstStageMask = &wait_stage,
                    .commandBufferCount = 1,
                    .pCommandBuffers = &buffer,
                    .signalSemaphoreCount = signal_semaphore_count,
                    .pSignalSemaphores = signal_semaphores,
                };

                vkResetFences(_device.lock()->handle(), 1, &node->fence);
                const VkResult submit_result = vkQueueSubmit(queue, 1, &submit_info, buffer_it->second->fence);
                node->is_submitted = true;
                return submit_result;
            }

            void command_pool::update() noexcept
            {
                process_completed_buffers_();
            }

            void command_pool::process_completed_buffers_() noexcept
            {
                auto it = _active_buffers.begin();
                while (it != _active_buffers.end())
                {
                    buffer_node* node = it->second;

                    if (node->is_submitted)
                    {
                        const VkResult fence_result = vkGetFenceStatus(_device.lock()->handle(), node->fence);
                        if (fence_result == VK_SUCCESS)
                        {
                            // logger::info("Returning to free list");
                            return_to_free_list_(node);
                            it = _active_buffers.erase(it);
                            continue;
                        }
                    }

                    ++it;
                }
            }

            void command_pool::return_to_free_list_(buffer_node* node) noexcept
            {
                node->next = _free_list_head;
                _free_list_head = node;
                node->is_submitted = false;
            }

            bool command_pool::allocate_buffers(const u32 num_buffers) noexcept
            {
//                if (!_buffers.empty())
//                {
//                    return false;
//                }

                VkCommandBufferAllocateInfo alloc_info {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    .commandPool = _command_pool,
                    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    .commandBufferCount = num_buffers
                };

                _buffers.resize(num_buffers);

                const VkResult result = vkAllocateCommandBuffers(
                    _device.lock()->handle()
                    , &alloc_info
                    , _buffers.data()
                );

                if (result != VK_SUCCESS)
                {
                    return false;
                }

                for (auto buffer : _buffers)
                {
                    VkFence fence = create_fence_().value();
                    std::unique_ptr<buffer_node> node = std::make_unique<buffer_node>(buffer, 0, fence);
                    node->next = _free_list_head;
                    _free_list_head = node.get();
                    _all_buffers.push_back(std::move(node));
                    
                    _buffer_handlers.push_back(std::move(std::make_unique<command_buffer>(buffer)));
                }

                return true;
            }

            void command_pool::wait_for_command_buffer(VkCommandBuffer buffer) const noexcept
            {
                auto cb_it = _active_buffers.find(buffer);
                if (cb_it == _active_buffers.end())
                {
                    return;
                }
                
                buffer_node* node = cb_it->second;
                if (node->is_submitted)
                {
                    return;
                }

                vkWaitForFences(_device.lock()->handle(), 1, &node->fence, VK_TRUE, UINT64_MAX);
            }

            void command_pool::reset_command_buffer_fence(VkCommandBuffer buffer) const noexcept
            {
                auto cb_it = _active_buffers.find(buffer);
                if (cb_it == _active_buffers.end())
                {
                    return;
                }

                buffer_node* node = cb_it->second;
                vkResetFences(_device.lock()->handle(), 1, &node->fence);
            }

            std::optional<VkFence> command_pool::create_fence_() const noexcept
            {
                VkFenceCreateInfo fence_info 
                {
                    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                    .flags = VK_FENCE_CREATE_SIGNALED_BIT
                };
                VkFence fence {};

                const VkResult result = vkCreateFence(_device.lock()->handle(), &fence_info, _allocation_callbacks, &fence);
                if (result != VK_SUCCESS)
                {
                    return std::nullopt;
                }

                return fence;
            }

            void command_pool::destroy() noexcept
            {
                logger::info("TOTAL BUFFERS: {}", _all_buffers.size());
                for (const auto& node : _all_buffers)
                {
                    vkDestroyFence(_device.lock()->handle(), node->fence, _allocation_callbacks);
                }
                vkFreeCommandBuffers(_device.lock()->handle(), _command_pool, _buffers.size(), _buffers.data());
                vkDestroyCommandPool(_device.lock()->handle(), _command_pool, _allocation_callbacks);
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace
