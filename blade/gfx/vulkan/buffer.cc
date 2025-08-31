#include "gfx/vulkan/buffer.h"
#include "core/logger.h"
#include "core/memory.h"
#include <cstring>
#include <exception>
#include <optional>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            vertex_buffer::builder::builder(std::weak_ptr<class device> device) noexcept
                : info { device }
            {}

            vertex_buffer::builder& vertex_buffer::builder::set_size(u32 size) noexcept
            {
                info.size = size;

                return *this;
            }
            
            vertex_buffer::builder& vertex_buffer::builder::set_allocation_callbacks(VkAllocationCallbacks* callbacks) noexcept
            {
                info.allocation_callbacks = callbacks;

                return *this;
            }


            std::optional<std::shared_ptr<vertex_buffer>> vertex_buffer::builder::build() const noexcept
            {
                VkBuffer buffer {};
                VkBufferCreateInfo create_info {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .size = info.size,
                    .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE
                };

                const VkResult result = vkCreateBuffer(
                    info.device.lock()->handle()
                    , &create_info
                    , info.allocation_callbacks
                    , &buffer
                );

                if (result != VK_SUCCESS)
                {
                    logger::error("FAILED TO CREATE VERTEX BUFFER");
                    return std::nullopt;
                }

                return std::make_shared<vertex_buffer>(buffer, info.size, info.device, info.allocation_callbacks);
            }

            void vertex_buffer::set_input_binding_description(VkVertexInputBindingDescription description) noexcept
            {
                _binding_description = description;
            }

            void vertex_buffer::add_input_attribute_description(VkVertexInputAttributeDescription desc) noexcept
            {
                _attribute_descriptions.push_back(desc);
            }




            vertex_buffer::vertex_buffer(VkBuffer buffer, u32 size, std::weak_ptr<class device> device, VkAllocationCallbacks* callbacks) noexcept
                : _buffer{ buffer }
                , _size{ size }
                , _device{ device }
                , _allocation_callbacks{ callbacks }
            {}

            void vertex_buffer::destroy() noexcept
            {
                vkDestroyBuffer(_device.lock()->handle(), _buffer, _allocation_callbacks);
                vkFreeMemory(_device.lock()->handle(), _memory, _allocation_callbacks);
            }

            void vertex_buffer::allocate() noexcept
            {
                VkMemoryRequirements requirements {};
                vkGetBufferMemoryRequirements(_device.lock()->handle(), _buffer, &requirements);

                VkMemoryAllocateInfo allocation_info {
                    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                    .pNext = nullptr,
                    .allocationSize = requirements.size,
                    .memoryTypeIndex = _device.lock()->get_physical_device().lock()->find_memory_type(
                            requirements.memoryTypeBits, 
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                        ).value(),
                };

                const VkResult result = vkAllocateMemory(_device.lock()->handle(), &allocation_info, _allocation_callbacks, &_memory);
                if (result != VK_SUCCESS)
                {
                    logger::error("FAILED TO ALLOCATE BUFFER");
                    std::terminate();
                }

                u32 offset { 0 };
                vkBindBufferMemory(_device.lock()->handle(), _buffer, _memory, offset);
                logger::info("Vertex buffer allocated and bound.");
            }

            void vertex_buffer::map_memory(void* memory) noexcept
            {
                void* data;
                u32 offset { 0 };
                VkMemoryMapFlags flags { 0 };
                vkMapMemory(_device.lock()->handle(), _memory, offset, _size, flags, &data);
                memcpy(data, memory, _size);
                logger::debug("Mapped {} bytes to vertex buffer", _size);
            }
        }
    } // gfx namespace
} // blade namespace
