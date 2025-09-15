#include "gfx/vulkan/shader.h"
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            std::optional<shader> shader::builder::build() const noexcept
            {
                shader shader_module{info.device};

                if (info.code.size() == 0)
                {
                    return std::nullopt;
                }

                VkShaderModuleCreateInfo create_info{};
                create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                create_info.codeSize = info.code.size();
                create_info.pCode = reinterpret_cast<const u32*>(info.code.data());

                const VkResult result = vkCreateShaderModule(
                    info.device.handle()
                    , &create_info
                    , info.callbacks
                    , &shader_module._shader_module
                );

                if (result != VK_SUCCESS)
                {
                    return std::nullopt;
                }

                return shader_module;
            }

            shader::builder& shader::builder::use_allocation_callbacks(VkAllocationCallbacks* callbacks) noexcept
            {
                info.callbacks = callbacks;

                return *this;
            }

            shader::builder& shader::builder::set_code(const std::vector<u8>& code) noexcept
            {
                info.code = code;

                return *this;
            }

            void shader::destroy() noexcept
            {
                vkDestroyShaderModule(_device.handle(), handle(), _callbacks);
            }
        }
    } // gfx namespace
} // blade namespace
