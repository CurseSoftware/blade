#include "gfx/vulkan/command.h"
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            ////////////////////////////////////////////////
            ///               RENDERPASS                 ///
            ////////////////////////////////////////////////
            void command_buffer::reset() const noexcept
            {
                const u32 flags = 0;
                vkResetCommandBuffer(_buffer, flags);
            }

            void command_buffer::end() const noexcept
            {
                vkEndCommandBuffer(_buffer);
            }

            void command_buffer::submit(const std::vector<VkSemaphore>& semaphores) const noexcept
            {
            }

            std::optional<command_buffer::recording> command_buffer::begin(VkCommandBufferUsageFlags flags) noexcept
            {
                VkCommandBufferBeginInfo begin_info {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .flags = flags
                };

                return recording::create(*this, begin_info);
            }
            
            std::optional<command_buffer::recording> command_buffer::recording::create(command_buffer& cb, VkCommandBufferBeginInfo begin_info) noexcept
            {
                auto rec = recording(cb);

                const VkResult result = vkBeginCommandBuffer(cb.handle(), &begin_info);
                if (result != VK_SUCCESS)
                {
                    return std::nullopt;
                }

                cb.is_recording = true;
                
                return rec;
            }

            command_buffer::recording::record_renderpass command_buffer::recording::begin_renderpass(std::weak_ptr<renderpass> rp, VkFramebuffer framebuffer,  const std::vector<VkClearValue>& clear_values, VkRect2D render_area) noexcept
            {
                return record_renderpass(*this, rp, framebuffer, clear_values, render_area);
            }

            ////////////////////////////////////////////////
            ///               RENDERPASS                 ///
            ////////////////////////////////////////////////

            void command_buffer::recording::record_renderpass::bind_pipeline(VkPipelineBindPoint bind_point, VkPipeline pipeline) const noexcept
            {
                vkCmdBindPipeline(_recording._buffer.handle(), bind_point, pipeline);
            }

            void command_buffer::recording::record_renderpass::set_viewport(VkViewport viewport) const noexcept
            {
                const u32 viewport_count = 1;
                const u32 first_viewport = 0;
                vkCmdSetViewport(_recording._buffer.handle(), first_viewport, viewport_count, &viewport);
            }

            void command_buffer::recording::record_renderpass::set_scissor(VkRect2D scissor) const noexcept
            {
                const u32 scissor_count = 1;
                const u32 first_scissor = 0;
                vkCmdSetScissor(_recording._buffer.handle(), first_scissor, scissor_count, &scissor);
            }

            void command_buffer::recording::record_renderpass::draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) const noexcept
            {
                vkCmdDraw(_recording._buffer.handle(), vertex_count, instance_count, first_vertex, first_instance);
            }

            bool command_buffer::recording::record_renderpass::end() noexcept
            {
                if (!_active)
                {
                    logger::info("inactive render pass");
                    return false;
                }

                vkCmdEndRenderPass(_recording._buffer.handle());
                _active = false;

                return true;
            }
            
            ////////////////////////////////////////////////
            ///              END RENDERPASS              ///
            ////////////////////////////////////////////////
        } // vk namespace
    } // gfx namespace
} // blade namespace
