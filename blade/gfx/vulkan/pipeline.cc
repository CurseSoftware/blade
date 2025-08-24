#include "gfx/vulkan/pipeline.h"
#include "gfx/vulkan/device.h"
#include <array>
#include <memory>
#include <optional>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            std::optional<std::shared_ptr<pipeline>> pipeline::builder::build() const noexcept
            {
                auto pipeline = std::make_shared<class pipeline>(info.device);


                VkPipelineDynamicStateCreateInfo dynamic_state {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                    .flags = 0,
                    .dynamicStateCount = static_cast<u32>(info.dynamic_states.size()),
                    .pDynamicStates = info.dynamic_states.data(),
                };

                VkPipelineVertexInputStateCreateInfo vertex_input {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                    .vertexBindingDescriptionCount = 0,
                    .pVertexBindingDescriptions = nullptr,
                    .vertexAttributeDescriptionCount = 0,
                    .pVertexAttributeDescriptions = nullptr
                };

                VkPipelineInputAssemblyStateCreateInfo input_assembly {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                    .primitiveRestartEnable = VK_FALSE
                };

                if (VK_SUCCESS != vkCreatePipelineLayout(
                    info.device.lock()->handle(), 
                    &info.pipeline_layout_info, 
                    info.allocation_callbacks, 
                    &pipeline->_layout))
                {
                    return std::nullopt;
                }

                switch(info.type)
                {
                    case type::compute:
                    {
                        // TODO: Not supported currently
                        return std::nullopt;
                    } break;
                    case type::graphics:
                    {
                        VkGraphicsPipelineCreateInfo graphics_info {
                            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                            .stageCount = static_cast<u32>(info.shader_stages.size()),
                            .pStages = info.shader_stages.data(),
                            .pVertexInputState = &info.vertex_info,
                            .pInputAssemblyState = &info.input_assembly_info,
                            .pViewportState = &info.viewport_info,
                            .pRasterizationState = &info.rasterization_info,
                            .pMultisampleState = &info.multisampler_info,
                            .pDepthStencilState = nullptr,
                            .pColorBlendState = &info.color_blend_info,
                            .pDynamicState = &dynamic_state,
                            .layout = pipeline->_layout,
                            .renderPass = info.renderpass,
                            .subpass = 0,
                        };

                        const VkResult graphics_result = vkCreateGraphicsPipelines(
                            info.device.lock()->handle()
                            , VK_NULL_HANDLE
                            , 1
                            , &graphics_info
                            , info.allocation_callbacks
                            , &pipeline->_pipeline
                        );

                        if (graphics_result != VK_SUCCESS)
                        {
                            return std::nullopt;
                        }
                    } break;
                }

                return pipeline;
            }

            pipeline::builder& pipeline::builder::add_dynamic_state(const VkDynamicState dynamic_state) noexcept
            {
                info.dynamic_states.push_back(dynamic_state);

                return *this;
            }

            pipeline::builder& pipeline::builder::set_type(const enum type type) noexcept
            {
                info.type = type;

                return *this;
            }

            pipeline::builder& pipeline::builder::add_viewport(const VkViewport viewport) noexcept
            {
                info.viewports.push_back(viewport);
                info.viewport_info.viewportCount = static_cast<u32>(info.viewports.size());
                info.viewport_info.pViewports = info.viewports.data();
                
                return *this;
            }
            
            pipeline::builder& pipeline::builder::add_scissor(const VkRect2D scissor) noexcept
            {
                info.scissors.push_back(scissor);
                info.viewport_info.scissorCount = static_cast<u32>(info.scissors.size());
                info.viewport_info.pScissors = info.scissors.data();
                
                return *this;
            }

            pipeline::builder& pipeline::builder::add_shader(shader::type type, const VkShaderModule& shader) noexcept
            {
                const VkShaderStageFlagBits stage = [type]() -> VkShaderStageFlagBits {
                    switch (type)
                    {
                        case shader::type::vertex:
                            return VK_SHADER_STAGE_VERTEX_BIT;
                        case shader::type::fragment:
                            return VK_SHADER_STAGE_FRAGMENT_BIT;
                        case shader::type::compute:
                            return VK_SHADER_STAGE_COMPUTE_BIT;
                    }
                }();
                
                VkPipelineShaderStageCreateInfo stage_info {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = stage,
                    .module = shader,
                    .pName = "main",
                };

                info.shader_stages.push_back(stage_info);
                
                return *this;
            }

            // pipeline::builder& pipeline::builder::add_multisampling(VkPipeline
            
            pipeline::builder& pipeline::builder::add_renderpass(const VkRenderPass& renderpass) noexcept
            {
                info.renderpass = renderpass;

                return *this;
            }
            
            pipeline::builder& pipeline::builder::use_allocation_callbacks(VkAllocationCallbacks* callbacks) noexcept
            {
                info.allocation_callbacks = callbacks;

                return *this;
            }
            
            pipeline::builder& pipeline::builder::set_extent(VkExtent2D extent) noexcept
            {
                info.extent = extent;

                return *this;
            }

            pipeline::builder& pipeline::builder::add_vertex_input_binding_description(const VkVertexInputBindingDescription binding_descriptions) noexcept
            {
                info.vertex_binding_descriptions.push_back(binding_descriptions);

                return *this;
            }
            
            pipeline::builder& pipeline::builder::add_vertex_input_attribute_description(const VkVertexInputAttributeDescription attribute_descriptions) noexcept
            {
                info.vertex_attribute_descriptions.push_back(attribute_descriptions);

                return *this;
            }
            
            pipeline::builder& pipeline::builder::set_input_assembly_topology(const VkPrimitiveTopology topology) noexcept
            {
                info.input_assembly_info.topology = topology;

                return *this;
            }
            
            pipeline::builder& pipeline::builder::set_input_assembly_primitive_restart(const VkBool32 restart) noexcept
            {
                info.input_assembly_info.primitiveRestartEnable = restart;
                info.input_assembly_info.flags = 0;

                return *this;
            }

            pipeline::builder& pipeline::builder::set_input_assembly_flags(const VkPipelineInputAssemblyStateCreateFlags flags) noexcept
            {
                info.input_assembly_info.flags |= flags;

                return *this;
            }
            
            pipeline::builder& pipeline::builder::set_input_assembly_pnext(const void* p_next) noexcept
            {
                info.input_assembly_info.pNext = p_next;

                return *this;
            }

            pipeline::builder& pipeline::builder::set_rasterization_pnext(const void* p_next) noexcept
            {
                info.rasterization_info.pNext = p_next;

                return *this;
            }

            pipeline::builder& pipeline::builder::set_rasterization_polygon_mode(const VkPolygonMode polygon) noexcept
            {
                info.rasterization_info.polygonMode = polygon;

                return *this;
            }
            
            pipeline::builder& pipeline::builder::set_rasterization_depth_clamp(const VkBool32 clamp) noexcept
            {
                info.rasterization_info.depthClampEnable = clamp;

                return *this;
            }
            
            pipeline::builder& pipeline::builder::set_rasterization_discard_enable(const VkBool32 discard_enable) noexcept
            {
                info.rasterization_info.rasterizerDiscardEnable = discard_enable;

                return *this;
            }
            
            pipeline::builder& pipeline::builder::set_rasterization_cull_mode(const VkCullModeFlags cull_mode) noexcept
            {
                info.rasterization_info.cullMode = cull_mode;
                
                return *this;
            }
            
            pipeline::builder& pipeline::builder::set_rasterization_front_face(const VkFrontFace front_face) noexcept
            {
                info.rasterization_info.frontFace = front_face;

                return *this;
            }
            
            pipeline::builder& pipeline::builder::set_rasterization_depth_bias(const VkBool32 depth_bias) noexcept
            {
                info.rasterization_info.depthBiasEnable = depth_bias;

                return *this;
            }
            
            pipeline::builder& pipeline::builder::set_rasterization_depth_bias_constant_factor(const f32 constant_factor) noexcept
            {
                info.rasterization_info.depthBiasConstantFactor = constant_factor;

                return *this;
            }
            
            pipeline::builder& pipeline::builder::set_rasterization_depth_bias_clamp(const f32 bias_clamp) noexcept
            {
                info.rasterization_info.depthBiasClamp = bias_clamp;

                return *this;
            }
            
            pipeline::builder& pipeline::builder::set_rasterization_depth_bias_slope_factor(const f32 slope_factor) noexcept
            {
                info.rasterization_info.depthBiasSlopeFactor = slope_factor;

                return *this;
            }
            
            pipeline::builder& pipeline::builder::set_rasterization_line_width(const f32 line_width) noexcept
            {
                info.rasterization_info.lineWidth = line_width;

                return *this;
            }
                            
            pipeline::builder& pipeline::builder::set_multisampler_rasterization_samples(const VkSampleCountFlagBits samples) noexcept
            {
                info.multisampler_info.rasterizationSamples = samples;

                return *this;
            }
            
            pipeline::builder& pipeline::builder::set_multisampler_sample_shading(const VkBool32 shading) noexcept
            {
                info.multisampler_info.sampleShadingEnable = shading;

                return *this;
            }
            
            pipeline::builder& pipeline::builder::add_multisampler_sample_mask(const VkSampleMask* mask) noexcept
            {
                info.multisampler_info.pSampleMask = mask;

                return *this;
            }
            
            pipeline::builder& pipeline::builder::add_multisampler_alpha_to_coverage(const VkBool32 coverage) noexcept
            {
                info.multisampler_info.alphaToCoverageEnable = coverage;

                return *this;
            }
            
            pipeline::builder& pipeline::builder::add_multisampler_alpha_to_one(const VkBool32 alpha) noexcept
            {
                info.multisampler_info.alphaToOneEnable = alpha;

                return *this;
            }

            pipeline::builder& pipeline::builder::set_multisampler_pnext(const void* p_next) noexcept
            {
                return *this;
            }

            pipeline::builder& pipeline::builder::add_pipeline_layout_push_constant(const VkPushConstantRange push_constant) noexcept
            {
                info.push_constants.push_back(push_constant);
                info.pipeline_layout_info.pushConstantRangeCount = static_cast<u32>(info.push_constants.size());
                info.pipeline_layout_info.pPushConstantRanges = info.push_constants.data();
                
                return *this;
            }
            
            pipeline::builder& pipeline::builder::add_pipeline_layout_descriptor_set(const VkDescriptorSetLayout descriptor_set) noexcept
            {
                info.descriptor_sets.push_back(descriptor_set);
                info.pipeline_layout_info.setLayoutCount = static_cast<u32>(info.descriptor_sets.size());
                info.pipeline_layout_info.pSetLayouts = info.descriptor_sets.data();

                return *this;
            }

            pipeline::builder& pipeline::builder::set_pipeline_layout_pnext(const void* p_next) noexcept
            {
                info.pipeline_layout_info.pNext = p_next;
                
                return *this;
            }

            pipeline::builder& pipeline::builder::set_pipeline_layout_flags(const VkPipelineLayoutCreateFlags flags) noexcept
            {
                info.pipeline_layout_info.flags |= flags;

                return *this;
            }

            void pipeline::destroy() noexcept
            {
                vkDestroyPipeline(_device.lock()->handle(), _pipeline, _allocation_callbacks);
                vkDestroyPipelineLayout(_device.lock()->handle(), _layout, _allocation_callbacks);
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace
