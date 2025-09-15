#include "gfx/vulkan/command_handler.h"
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            command_handler::command_handler(std::weak_ptr<class device> device, const queue_type queue) noexcept
                : _device{device}
            {
                constexpr u32 num_buffers = 16;

                auto const transfer_pool_opt = command_pool::builder(device)
                                               .use_allocation_callbacks(nullptr)
                                               .set_queue_family_index(
                                                   device.lock()->get_queue_index(queue).value())
                                               .build();
                _command_pool = transfer_pool_opt.value();
                _all_command_buffers = _command_pool->allocate_buffers(num_buffers);

                for (VkCommandBuffer buffer : _all_command_buffers)
                {
                    VkFence fence = create_fence_().value();
                    auto node = std::make_unique<buffer_free_list::node>(
                        buffer, fence);
                    _free_list.push_front(node.get());
                    _all_buffer_nodes.push_back(std::move(node));
                }
            }

            VkCommandBuffer command_handler::acquire_command_buffer() noexcept
            {
                if (_free_list.is_empty())
                {
                    return VK_NULL_HANDLE;
                }

                buffer_free_list::node* node = _free_list.pop_front();

                constexpr u32 flags = 0;
                vkResetCommandBuffer(node->command_buffer, flags);

                node->is_submitted = false;
                _active_nodes[node->command_buffer] = node;

                return node->command_buffer;
            }

            void command_handler::update() noexcept
            {
                process_completed_buffers_();
            }

            std::optional<VkFence> command_handler::create_fence_() const noexcept
            {
                VkFenceCreateInfo fence_info
                {
                    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                    .flags = VK_FENCE_CREATE_SIGNALED_BIT
                };
                VkFence fence{};

                VkAllocationCallbacks* callbacks{nullptr};
                const VkResult result = vkCreateFence(_device.lock()->handle(), &fence_info, callbacks, &fence);
                if (result != VK_SUCCESS)
                {
                    return std::nullopt;
                }

                return fence;
            }

            VkResult command_handler::submit_buffer(
                VkCommandBuffer buffer
                , VkQueue queue
                , const VkSemaphore* wait_semaphores
                , u32 wait_semaphore_count
                , const VkSemaphore* signal_semaphores
                , u32 signal_semaphore_count
                , VkPipelineStageFlags wait_stage
            ) const noexcept
            {
                auto buffer_it = _active_nodes.find(buffer);
                if (buffer_it == _active_nodes.end())
                {
                    return VK_ERROR_UNKNOWN;
                }

                buffer_free_list::node* node = buffer_it->second;

                const VkSubmitInfo submit_info{
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


            void command_handler::process_completed_buffers_() noexcept
            {
                auto it = _active_nodes.begin();
                while (it != _active_nodes.end())
                {
                    buffer_free_list::node* node = it->second;
                    if (node->is_submitted)
                    {
                        const VkResult fence_result = vkGetFenceStatus(
                            _device.lock()->handle()
                            , node->fence
                        );

                        if (fence_result == VK_SUCCESS)
                        {
                            _free_list.push_front(node);
                            it = _active_nodes.erase(it);
                            continue;
                        }
                    }

                    ++it;
                }
            }

            void command_handler::wait_for_command_buffer(VkCommandBuffer buffer) const noexcept
            {
                auto cb_it = _active_nodes.find(buffer);
                if (cb_it == _active_nodes.end())
                {
                    return;
                }

                buffer_free_list::node* node = cb_it->second;
                if (node->is_submitted)
                {
                    return;
                }

                vkWaitForFences(_device.lock()->handle(), 1, &node->fence, VK_TRUE, UINT64_MAX);
            }

            void command_handler::reset_command_buffer_fence(VkCommandBuffer buffer) const noexcept
            {
                auto cb_it = _active_nodes.find(buffer);
                if (cb_it == _active_nodes.end())
                {
                    return;
                }

                buffer_free_list::node* node = cb_it->second;
                vkResetFences(_device.lock()->handle(), 1, &node->fence);
            }

            void command_handler::destroy() const noexcept
            {
                const VkAllocationCallbacks* callbacks{nullptr};
                logger::info("TOTAL BUFFERS: {}", _all_buffer_nodes.size());
                for (const auto& node : _all_buffer_nodes)
                {
                    vkDestroyFence(_device.lock()->handle(), node->fence, callbacks);
                }
                vkFreeCommandBuffers(_device.lock()->handle(), _command_pool->handle(), _all_command_buffers.size(),
                                     _all_command_buffers.data());
                // vkDestroyCommandPool(_device.lock()->handle(), _command_pool->handle(), callbacks);
                _command_pool->destroy();
            }


            ////////////////////////////////////////////////
            ///             FREE LIST IMPL               ///
            ////////////////////////////////////////////////

            bool command_handler::buffer_free_list::is_empty() const noexcept
            {
                return _front == nullptr;
            }

            void command_handler::buffer_free_list::push_front(node* buffer_node) noexcept
            {
                buffer_node->next = _front;
                _front = buffer_node;
                buffer_node->is_submitted = false;
            }

            command_handler::buffer_free_list::node* command_handler::buffer_free_list::pop_front() noexcept
            {
                node* buffer_node = _front;
                _front = buffer_node->next;
                return buffer_node;
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace
