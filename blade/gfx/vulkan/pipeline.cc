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
                std::array<VkDynamicState, 2> dynamic_states = {
                    VK_DYNAMIC_STATE_VIEWPORT
                    , VK_DYNAMIC_STATE_SCISSOR
                };

                // class pipeline pipeline { info.device };
                auto pipeline = std::make_shared<class pipeline>(info.device);


                VkPipelineDynamicStateCreateInfo dynamic_state {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                    .flags = 0,
                    .dynamicStateCount = static_cast<u32>(dynamic_states.size()),
                    .pDynamicStates = dynamic_states.data(),
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

                VkViewport viewport {
                    .x = 0.0f,
                    .y = 0.0f,
                    .width = static_cast<float>(info.extent.width),
                    .height = static_cast<float>(info.extent.height),
                    .minDepth = 0.0f,
                    .maxDepth = 1.0f
                };

                VkRect2D scissor {
                    .offset = { 0, 0 },
                    .extent = info.extent
                };

                VkPipelineViewportStateCreateInfo viewport_state {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                    .viewportCount = 1,
                    .pViewports = &viewport,
                    .scissorCount = 1,
                    .pScissors = &scissor
                };

                VkPipelineRasterizationStateCreateInfo rasterizer {
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

                VkPipelineMultisampleStateCreateInfo multisampling {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                    .sampleShadingEnable = VK_FALSE,
                    .minSampleShading = 1.0f,
                    .pSampleMask = nullptr,
                    .alphaToCoverageEnable = VK_FALSE,
                    .alphaToOneEnable = VK_FALSE,
                };

                VkPipelineColorBlendAttachmentState color_blend_attachment {
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

                VkPipelineColorBlendStateCreateInfo color_blending {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                    .logicOpEnable = VK_FALSE,
                    .logicOp = VK_LOGIC_OP_COPY,
                    .attachmentCount = 1,
                    .pAttachments = &color_blend_attachment,
                    .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f },
                };

                VkPipelineLayoutCreateInfo pipeline_layout_info {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                    .setLayoutCount = 0,
                    .pSetLayouts = nullptr,
                    .pushConstantRangeCount = 0,
                    .pPushConstantRanges = nullptr,
                };

                if (VK_SUCCESS != vkCreatePipelineLayout(info.device.lock()->handle(), &pipeline_layout_info, info.allocation_callbacks, &pipeline->_layout))
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
                            .pVertexInputState = &vertex_input,
                            .pInputAssemblyState = &input_assembly,
                            .pViewportState = &viewport_state,
                            .pRasterizationState = &rasterizer,
                            .pMultisampleState = &multisampling,
                            .pDepthStencilState = nullptr,
                            .pColorBlendState = &color_blending,
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

            pipeline::builder& pipeline::builder::set_type(const enum type type) noexcept
            {
                info.type = type;

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
            
            void pipeline::destroy() noexcept
            {
                vkDestroyPipeline(_device.lock()->handle(), _pipeline, _allocation_callbacks);
                vkDestroyPipelineLayout(_device.lock()->handle(), _layout, _allocation_callbacks);
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace
