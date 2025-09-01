#ifndef BLADFE_GFX_VULKAN_BUFFER_H
#define BLADFE_GFX_VULKAN_BUFFER_H
#include "core/memory.h"
#include "gfx/vulkan/command.h"
#include "gfx/vulkan/common.h"
#include "gfx/vulkan/device.h"
#include <vector>
#include <memory>
#include <optional>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            class buffer
            {
                public:
                    struct builder
                    {
                        [[nodiscard]] explicit builder(std::weak_ptr<class device> device) noexcept;
                        std::optional<std::shared_ptr<buffer>> build() const noexcept;

                        builder& set_usage(const VkBufferUsageFlags usage) noexcept;
                        builder& set_size(const u32 size) noexcept;
                        builder& set_allocation_callbacks(VkAllocationCallbacks* callbacks) noexcept;
                        builder& set_sharing_mode(const VkSharingMode sharing_mode) noexcept;

                        struct
                        {
                            std::weak_ptr<class device> device;
                            VkBufferUsageFlags usage                    { VK_BUFFER_USAGE_VERTEX_BUFFER_BIT };
                            u32 size                                    { 0 };
                            VkAllocationCallbacks* allocation_callbacks { nullptr };
                            VkSharingMode sharing_mode                  { VK_SHARING_MODE_EXCLUSIVE };
                        } info;
                    };

                    [[nodiscard]] explicit buffer(VkBuffer buffer, u32 size, std::weak_ptr<class device> device, VkAllocationCallbacks* callbacks) noexcept;

                    VkVertexInputBindingDescription binding_description() const noexcept { return _binding_description; }
                    const std::vector<VkVertexInputAttributeDescription>& attribute_descriptions() const noexcept { return _attribute_descriptions; }

                    [[nodiscard]] const VkBuffer& handle() const noexcept { return _buffer; }
                    [[nodiscard]] VkBuffer* handle_ptr() noexcept { return &_buffer; }
                    
                    void set_input_binding_description(VkVertexInputBindingDescription desc) noexcept;
                    void add_input_attribute_description(VkVertexInputAttributeDescription desc) noexcept;

                    void allocate(VkMemoryPropertyFlags flags) noexcept;

                    void destroy() noexcept;

                    void map_memory(void* memory) noexcept;

                    [[nodiscard]] u32 size() const noexcept { return _size; }

                private:
                    
                    VkBuffer _buffer                                                       {};
                    u32 _size                                                              { 0 };
                    std::weak_ptr<class device> _device                                    {};
                    VkDeviceMemory _memory                                                 {};
                    VkAllocationCallbacks* _allocation_callbacks                           { nullptr };
                    
                    VkVertexInputBindingDescription _binding_description                   {};
                    std::vector<VkVertexInputAttributeDescription> _attribute_descriptions {};
            };
        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADFE_GFX_VULKAN_BUFFER_H
