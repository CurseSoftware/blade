#ifndef BLADE_GFX_VULKAN_RENDERPASS_H
#define BLADE_GFX_VULKAN_RENDERPASS_H

#include "gfx/vulkan/common.h"
#include "gfx/vulkan/device.h"

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            enum class attachment_description_kind 
            {
                color
            };
            
            class renderpass
            {
                public:
                    struct builder
                    {
                        [[nodiscard]] explicit builder(std::weak_ptr<class device> device) noexcept
                            : info { device }
                        {}

                        std::optional<std::shared_ptr<renderpass>> build() const noexcept;

                        builder& use_allocation_callbacks(VkAllocationCallbacks* callbacks) noexcept;
                        builder& add_subpass_description(const VkSubpassDescription description) noexcept;
                        builder& add_attachment(const VkAttachmentDescription description) noexcept;
                        builder& add_subpass_dependency(VkSubpassDependency dependency) noexcept;

                        struct
                        {
                            std::weak_ptr<class device> device                     {};
                            VkFormat format                                        {};
                            VkAllocationCallbacks* allocation_callbacks            { nullptr };
                            std::vector<VkSubpassDescription> subpass_descriptions {};
                            std::vector<VkAttachmentDescription> attachments       {};
                            std::vector<VkSubpassDependency> subpass_dependencies {};
                        } info {};
                    };

                    const VkRenderPass& handle() const noexcept { return _renderpass; }

                    void destroy() const noexcept;
                
                    [[nodiscard]] explicit renderpass(std::weak_ptr<class device> device) noexcept
                        : _device{device}
                    {}
                private:

                    std::weak_ptr<class device> _device          {};
                    VkRenderPass _renderpass                     { VK_NULL_HANDLE };
                    VkAllocationCallbacks* _allocation_callbacks { nullptr };
            };

        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_RENDERPASS_H
