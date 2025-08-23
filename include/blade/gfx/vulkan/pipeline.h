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

                            std::optional<std::shared_ptr<pipeline>> build() noexcept;

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

                            builder& add_vertex_input_binding_description(const VkVertexInputBindingDescription) noexcept;
                            builder& add_vertex_input_attribute_description(const VkVertexInputAttributeDescription) noexcept;

                            builder& set_input_assembly_topology(const VkPrimitiveTopology) noexcept;
                            builder& set_input_assembly_primitive_restart(const VkBool32) noexcept;
                            builder& set_input_assembly_flags(const VkPipelineInputAssemblyStateCreateFlags) noexcept;
                            builder& set_input_assembly_pnext(const void*) noexcept;

                            builder& set_rasterization_depth_clamp(const VkBool32) noexcept; 
                            builder& set_rasterization_discard_enable(const VkBool32) noexcept; 
                            builder& set_rasterization_cull_mode(const VkCullModeFlags) noexcept;
                            builder& set_rasterization_front_face(const VkFrontFace) noexcept;
                            builder& set_rasterization_depth_bias(const VkBool32) noexcept;
                            builder& set_rasterization_depth_bias_constant_factor(const f32) noexcept;
                            builder& set_rasterization_depth_bias_clamp(const f32) noexcept;
                            builder& set_rasterization_depth_bias_slope_factor(const f32) noexcept;
                            builder& set_rasterization_line_width(const f32) noexcept;
                            builder& set_rasterization_pnext(const void*) noexcept;

                            builder& set_multisampler_rasterization_samples(const VkSampleCountFlagBits) noexcept;
                            builder& set_multisampler_sample_shading(const VkBool32) noexcept;
                            builder& add_multisampler_sample_mask(const VkSampleMask*) noexcept;
                            builder& add_multisampler_alpha_to_coverage(const VkBool32) noexcept;
                            builder& add_multisampler_alpha_to_one(const VkBool32) noexcept;
                            builder& set_multisampler_pnext(const void*) noexcept;

                            // TODO: make references?
                            builder& add_pipeline_layout_descriptor_set(const VkDescriptorSetLayout) noexcept;
                            builder& add_pipeline_layout_push_constant(const VkPushConstantRange) noexcept;
                            builder& set_pipeline_layout_pnext(const void*) noexcept;
                            builder& set_pipeline_layout_flags(const VkPipelineLayoutCreateFlags) noexcept;
                            
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
                                
                                std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions     {};
                                std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions {};

                                std::vector<VkDescriptorSetLayout> descriptor_sets {};
                                std::vector<VkPushConstantRange> push_constants    {};

                                VkPipelineVertexInputStateCreateInfo vertex_info           
                                { 
                                    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                                    .vertexBindingDescriptionCount = 0,
                                    .pVertexBindingDescriptions = nullptr,
                                    .vertexAttributeDescriptionCount = 0,
                                    .pVertexAttributeDescriptions = nullptr
                                };
                                
                                VkPipelineInputAssemblyStateCreateInfo input_assembly_info
                                { 
                                    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                                    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                                    .primitiveRestartEnable = VK_FALSE,
                                };
                                
                                VkPipelineViewportStateCreateInfo viewport_info            
                                { 
                                    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                                };
                                
                                VkPipelineRasterizationStateCreateInfo rasterization_info  
                                { 
                                    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                                    .depthClampEnable = VK_FALSE,
                                    .rasterizerDiscardEnable = VK_FALSE,
                                    .polygonMode = VK_POLYGON_MODE_FILL,
                                    .cullMode = VK_CULL_MODE_BACK_BIT,
                                    .frontFace = VK_FRONT_FACE_CLOCKWISE,
                                    .depthBiasEnable = VK_FALSE,
                                    .depthBiasConstantFactor = 0.0f,
                                    .depthBiasClamp = 0.0f,
                                    .depthBiasSlopeFactor = 0.0f,
                                    .lineWidth = 1.0f,
                                };
                                
                                VkPipelineMultisampleStateCreateInfo multisampler_info     
                                { 
                                    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                                    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                                    .sampleShadingEnable = VK_FALSE,
                                    .minSampleShading = 1.0f,
                                    .pSampleMask = nullptr,
                                    .alphaToCoverageEnable = VK_FALSE,
                                    .alphaToOneEnable = VK_FALSE,
                                };
                                
                                VkPipelineLayoutCreateInfo pipeline_layout_info            
                                {
                                    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                    .setLayoutCount = 0,
                                    .pSetLayouts = nullptr,
                                    .pushConstantRangeCount = 0,
                                    .pPushConstantRanges = nullptr,
                                };
                               
                                // TODO: better implementation of color blending
                                VkPipelineColorBlendAttachmentState color_blend_attachment 
                                {
                                    .blendEnable = VK_FALSE,
                                    .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                                    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                                    .colorBlendOp = VK_BLEND_OP_ADD,
                                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                                    .alphaBlendOp = VK_BLEND_OP_ADD,
                                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT
                                        | VK_COLOR_COMPONENT_G_BIT
                                        | VK_COLOR_COMPONENT_B_BIT
                                        | VK_COLOR_COMPONENT_A_BIT,
                                };
                                VkPipelineColorBlendStateCreateInfo color_blend_info       
                                { 
                                    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                                    .logicOpEnable = VK_FALSE,
                                    .logicOp = VK_LOGIC_OP_COPY,
                                    .attachmentCount = 1,
                                    .pAttachments = &color_blend_attachment,
                                    .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f },
                                };

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
