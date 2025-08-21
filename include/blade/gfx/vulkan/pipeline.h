#ifndef BLADE_GFX_VULKAN_PIPELINE_H
#define BLADE_GFX_VULKAN_PIPELINE_H

#include "gfx/vulkan/common.h"
#include "gfx/vulkan/shader.h"

#include <array>
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
                    enum type
                    {
                        compute,
                        graphics
                    };

                    class builder
                    {
                        public:
                            [[nodiscard]] explicit builder(std::weak_ptr<const class device> device) noexcept
                                : info { device }
                            {}

                            std::optional<std::shared_ptr<pipeline>> build() const noexcept;

                            builder& add_dynamic_state(const VkDynamicState dynamic_state) noexcept;
                            builder& set_type(const enum type type) noexcept;
                            builder& use_allocation_callbacks(VkAllocationCallbacks* callbacks) noexcept;
                            builder& use_blending(const bool enabled = true) noexcept;
                            builder& add_multisampling(const bool enabled = true) noexcept;
                            builder& add_renderpass(const VkRenderPass& renderpass) noexcept;
                            builder& add_shader(shader::type type, const VkShaderModule&) noexcept;
                            builder& set_render_pass(const VkRenderPass& renderpass) noexcept;
                            builder& set_pipeline_layout(const VkPipelineLayout& layout) noexcept;
                            builder& add_viewport(const VkViewport viewport) noexcept;
                            builder& add_scissor(const VkRect2D scissor) noexcept;
                            builder& set_extent(VkExtent2D extent) noexcept;
                            
                            struct
                            {
                                std::weak_ptr<const class device> device                   {};
                                std::vector<VkViewport> viewports                          {};
                                std::vector<VkRect2D> scissors                             {};
                                std::vector<VkPipelineShaderStageCreateInfo> shader_stages {};
                                std::vector<VkDynamicState> dynamic_states                 {};
                                VkAllocationCallbacks* allocation_callbacks                { nullptr };
                                VkExtent2D extent                                          {};
                                VkPipelineLayout pipeline_layout                           { VK_NULL_HANDLE };
                                VkShaderModule vertex_shader_module                        { VK_NULL_HANDLE };
                                VkShaderModule fragment_shader_module                      { VK_NULL_HANDLE };
                                enum type type                                             { type::graphics };
                                VkRenderPass renderpass                                    { VK_NULL_HANDLE };


                                // TODO: Add methods for enabling these
                                struct
                                {
                                    VkPolygonMode polygon_mode { VK_POLYGON_MODE_FILL };
                                    VkCullModeFlags cull_mode { VK_CULL_MODE_BACK_BIT };
                                    VkFrontFace front_face { VK_FRONT_FACE_CLOCKWISE };
                                    VkBool32 depth_bias_enable { VK_FALSE };
                                    f32 depth_bias_constant_factor { 0.0f };
                                    f32 depth_bias_clamp { 0.0f };
                                    f32 depth_bias_slope_factor { 0.0f };
                                    f32 line_width { 1.0f };
                                } rasterization {};

                                struct
                                {
                                    VkSampleCountFlags sample_count   { VK_SAMPLE_COUNT_1_BIT };
                                    VkBool32 sample_shading_enable    { VK_FALSE };
                                    f32 min_sample_shading            { 1.0f };
                                    VkSampleMask* sample_mask         { nullptr };
                                    VkBool32 alpha_to_coverage_enable { VK_FALSE };
                                    VkBool32 alpha_to_one_enable      { VK_FALSE };
                                } multisampling {};

                                struct
                                {
                                    VkBool32 enable { VK_FALSE };
                                    VkBlendFactor color_src_factor { VK_BLEND_FACTOR_SRC_ALPHA };
                                    VkBlendFactor color_dst_factor { VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA };
                                    VkBlendOp color_op { VK_BLEND_OP_ADD };
                                    VkBlendFactor alpha_src_factor { VK_BLEND_FACTOR_ONE };
                                    VkBlendFactor alpha_dst_factor { VK_BLEND_FACTOR_ZERO };
                                    VkBlendOp alpha_op { VK_BLEND_OP_ADD };
                                    VkColorComponentFlags write_mask {
                                        VK_COLOR_COMPONENT_R_BIT
                                        | VK_COLOR_COMPONENT_G_BIT
                                        | VK_COLOR_COMPONENT_B_BIT
                                        | VK_COLOR_COMPONENT_A_BIT
                                    };
                                    VkLogicOp logic_op { VK_LOGIC_OP_COPY };
                                    std::array<f32, 4> blend_constants { 0.0f, 0.0f, 0.0f, 0.0f };
                                } color_blending {};

                            } info {};
                    };
                
                // Public Methods
                public:
                    void destroy() noexcept;
                    VkPipeline handle() const noexcept { return _pipeline; }

                    [[nodiscard]] explicit pipeline(std::weak_ptr<const class device> device) noexcept
                        : _device{ device }
                    {}
                // Private Methods
                private:

                private:
                    std::weak_ptr<const class device> _device    {};
                    VkPipelineLayout _layout                     { VK_NULL_HANDLE };
                    VkPipeline _pipeline                         { VK_NULL_HANDLE };
                    VkAllocationCallbacks* _allocation_callbacks { nullptr };
            };
        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_PIPELINE_H
