#ifndef BLADE_GFX_VULKAN_PIPELINE_H
#define BLADE_GFX_VULKAN_PIPELINE_H

#include "gfx/vulkan/common.h"

#include <optional>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            class pipeline
            {
                public:

                    class builder
                    {
                        [[nodiscard]] explicit builder(std::weak_ptr<const class device> device) noexcept
                            : info { device }
                        {}

                        std::optional<pipeline> build() const noexcept;

                        builder& use_allocation_callbacks(VkAllocationCallbacks* callbacks) noexcept;
                        
                        struct
                        {
                            std::weak_ptr<const class device> device;
                            VkAllocationCallbacks* allocation_callbacks;
                            VkExtent2D extent;
                        } info;
                    };
                private:
                    [[nodiscard]] explicit pipeline(std::weak_ptr<const class device> device) noexcept
                        : _device{ device }
                    {}

                    void destroy() noexcept;

                private:
                    std::weak_ptr<const class device> _device;
                    VkPipelineLayout _layout { VK_NULL_HANDLE };
                    VkAllocationCallbacks* _allocation_callbacks { nullptr };
            };
        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_PIPELINE_H
