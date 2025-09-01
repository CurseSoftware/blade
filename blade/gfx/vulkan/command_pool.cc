#include "gfx/vulkan/command.h"
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

            bool command_pool::allocate_buffers(const u32 num_buffers) noexcept
            {
                if (!_buffers.empty())
                {
                    return false;
                }

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
                    _buffer_handlers.push_back(std::move(std::make_unique<command_buffer>(buffer)));
                }

                return true;
            }

            void command_pool::destroy() noexcept
            {
                vkDestroyCommandPool(_device.lock()->handle(), _command_pool, _allocation_callbacks);
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace
