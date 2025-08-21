#include "gfx/vulkan/renderpass.h"
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            std::optional<std::shared_ptr<renderpass>> renderpass::builder::build() const noexcept
            {
                auto renderpass = std::make_shared<class renderpass>(info.device);

                VkRenderPassCreateInfo renderpass_info {
                    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                    .attachmentCount = static_cast<u32>(info.attachments.size()),
                    .pAttachments = info.attachments.data(),
                    .subpassCount = static_cast<u32>(info.subpass_descriptions.size()),
                    .pSubpasses = info.subpass_descriptions.data(),
                    .dependencyCount = static_cast<u32>(info.subpass_dependencies.size()),
                    .pDependencies = info.subpass_dependencies.data()
                };

                const VkResult result = vkCreateRenderPass(
                    info.device.lock()->handle(), 
                    &renderpass_info, 
                    info.allocation_callbacks, 
                    &renderpass->_renderpass
                );

                renderpass->_allocation_callbacks = info.allocation_callbacks;

                if (result != VK_SUCCESS)
                {
                    return std::nullopt;
                }

                return renderpass;
            }

            renderpass::builder& renderpass::builder::add_subpass_dependency(VkSubpassDependency dependency) noexcept
            {
                info.subpass_dependencies.push_back(dependency);

                return *this;
            }

            renderpass::builder& renderpass::builder::use_allocation_callbacks(VkAllocationCallbacks* callbacks) noexcept
            {
                info.allocation_callbacks = callbacks;
                
                return *this;
            }

            renderpass::builder& renderpass::builder::add_attachment(const VkAttachmentDescription description) noexcept
            {
                info.attachments.push_back(description);

                return *this;
            }
            
            renderpass::builder& renderpass::builder::add_subpass_description(const VkSubpassDescription description) noexcept
            {
                info.subpass_descriptions.push_back(description);
                
                return *this;
            }

            void renderpass::destroy() const noexcept
            {
                vkDestroyRenderPass(_device.lock()->handle(), _renderpass, _allocation_callbacks);
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace
