#ifndef BLADFE_GFX_VULKAN_BUFFER_H
#define BLADFE_GFX_VULKAN_BUFFER_H
#include "core/memory.h"
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
            class vertex_buffer
            {
                public:
                    struct builder
                    {
                        [[nodiscard]] explicit builder(std::weak_ptr<class device> device) noexcept;
                        std::optional<std::shared_ptr<vertex_buffer>> build() const noexcept;

                        builder& set_size(u32 size) noexcept;
                        builder& set_allocation_callbacks(VkAllocationCallbacks* callbacks) noexcept;

                        struct
                        {
                            std::weak_ptr<class device> device;
                            u32 size                                    { 0 };
                            VkAllocationCallbacks* allocation_callbacks { nullptr };
                        } info;
                    };

                    [[nodiscard]] explicit vertex_buffer(VkBuffer buffer, u32 size, std::weak_ptr<class device> device, VkAllocationCallbacks* callbacks) noexcept;

                    VkVertexInputBindingDescription binding_description() const noexcept { return _binding_description; }
                    const std::vector<VkVertexInputAttributeDescription>& attribute_descriptions() const noexcept { return _attribute_descriptions; }
                    
                    void set_input_binding_description(VkVertexInputBindingDescription desc) noexcept;
                    void add_input_attribute_description(VkVertexInputAttributeDescription desc) noexcept;

                    void allocate() noexcept;

                    void destroy() noexcept;

                    void map_memory(void* memory) noexcept;

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
