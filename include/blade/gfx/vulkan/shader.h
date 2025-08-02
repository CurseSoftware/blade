#ifndef BLADE_GFX_VULKAN_SHADER_H
#define BLADE_GFX_VULKAN_SHADER_H

#include "core/types.h"
#include "gfx/vulkan/common.h"
#include "gfx/vulkan/device.h"
#include "resources/resources.h"

#include <vector>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            class shader
            {
                public:
                    enum type
                    {
                        vertex,
                        fragment,
                        compute
                    };

                    struct builder
                    {
                        [[nodiscard]] explicit builder(const class device& device) noexcept
                            : info { device }
                        {}

                        std::optional<shader> build() const noexcept;

                        builder& use_allocation_callbacks(VkAllocationCallbacks* callbacks) noexcept;
                        builder& set_code(const std::vector<u8>& code) noexcept;

                        struct
                        {
                            const class device& device;
                            VkAllocationCallbacks* callbacks { nullptr };
                            std::vector<u8> code;
                        } info;
                    };

                    /**
                     * @brief Create a shader directly from it's data. Normally used if the shader has been read from file or directly from code
                     * @return `std::unique_ptr<shader>` to the shader
                     */
                    static std::unique_ptr<shader> from(const class device& device, const std::vector<u8>& data) noexcept;
                   
                    /**
                     * @brief Create a shader from a file
                     * @return `std::unique_ptr<shader>` to the shader
                     */
                    static std::unique_ptr<shader> from_file(const resources::fs::file& file) noexcept;

                    /**
                     * @brief Get handle to inner shader module
                     * @return `VkShaderModule` handle
                     */
                    const VkShaderModule handle() const noexcept { return _shader_module; }
                    
                    /**
                     * @brief Get handle to shader stage info. Mainly used from the pipeline
                     * @return `VkPipelineShaderStageCreateInfo` handle
                     */
                    const VkPipelineShaderStageCreateInfo shader_stage() const noexcept { return _pipeline_info; }

                    /**
                     * @brief Destroy the shader module
                     */
                    void destroy() noexcept;
                private:
                    [[nodiscard]] explicit shader(const class device& device) noexcept 
                        : _device{device}
                    {}

                private:
                    std::vector<u8> _data {};
                    VkShaderModule _shader_module { VK_NULL_HANDLE };
                    VkPipelineShaderStageCreateInfo _pipeline_info {};
                    const class device& _device;
                    VkAllocationCallbacks* _callbacks { nullptr };
            };
        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_SHADER_H

