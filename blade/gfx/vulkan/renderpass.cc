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
                
                VkAttachmentDescription color_attachment {
                    .format = VK_FORMAT_B8G8R8A8_SRGB,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                };
                
                VkAttachmentReference color_attachment_reference {
                    .attachment = 0,
                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                };

                VkSubpassDescription subpass {
                    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                    .colorAttachmentCount = 1,
                    .pColorAttachments = &color_attachment_reference,
                };

                VkRenderPassCreateInfo renderpass_info {
                    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                    .attachmentCount = 1,
                    .pAttachments = &color_attachment,
                    .subpassCount = 1,
                    .pSubpasses = &subpass
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

            renderpass::builder& renderpass::builder::use_allocation_callbacks(VkAllocationCallbacks* callbacks) noexcept
            {
                info.allocation_callbacks = callbacks;
                
                return *this;
            }
            
            renderpass::builder& renderpass::builder::add_subpass_description(VkSubpassDescription description) noexcept
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
