#include "gfx/vulkan/command.h"
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            ////////////////////////////////////////////////
            ///             COMMAND BUFFER               ///
            ////////////////////////////////////////////////

            command_buffer::command_buffer(VkCommandBuffer buffer) noexcept
                : _buffer{ buffer }
            {}

            void command_buffer::reset() const noexcept
            {
                const u32 flags = 0;
                vkResetCommandBuffer(_buffer, flags);
            }

            void command_buffer::end() noexcept
            {
                vkEndCommandBuffer(_buffer);
                _is_active = false;
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

                _is_active = true;

                return recording::create(*this, begin_info);
            }
            
            ////////////////////////////////////////////////
            ///               RECORDING                 ///
            ////////////////////////////////////////////////

            command_buffer::recording::recording(command_buffer& cb) noexcept
                : _buffer { cb }
            {}

            command_buffer::recording::~recording() noexcept
            {
                _buffer.is_recording = false;
            }

            command_buffer::recording::recording(recording&& other) noexcept
                : _buffer{ other._buffer }
            {}

            command_buffer::recording& command_buffer::recording::operator=(recording&& other) noexcept
            {
                if (this != &other)
                {
                    _buffer = other._buffer;
                }

                return *this;
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
            
            ////////////////////////////////////////////////
            ///               RENDERPASS                 ///
            ////////////////////////////////////////////////
            command_buffer::recording::record_renderpass::record_renderpass(
                    recording& rec
                    , std::weak_ptr<renderpass> rp
                    , VkFramebuffer framebuffer
                    , const std::vector<VkClearValue>& clear_values
                    , VkRect2D render_area
                    ) noexcept
                : _recording{ rec }
                , _renderpass{ rp }
                , _active{ true }
            {
                VkRenderPassBeginInfo pass_info {
                    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                        .renderPass = _renderpass.lock()->handle(),
                        .framebuffer = framebuffer,
                        .renderArea = render_area,
                        .clearValueCount = static_cast<u32>(clear_values.size()),
                        .pClearValues = clear_values.data()
                };

                vkCmdBeginRenderPass(rec._buffer.handle(), &pass_info, VK_SUBPASS_CONTENTS_INLINE);
            }

            command_buffer::recording::record_renderpass::~record_renderpass() noexcept
            {
                if (_active)
                {
                    logger::warn("Renderpass Recording destroying without being submitted");
                }
            }
            command_buffer::recording::record_renderpass::record_renderpass(record_renderpass&& other) noexcept
                : _recording{ other._recording }
                , _renderpass{ other._renderpass }
                , _active{ other._active }
            {}

            command_buffer::recording::record_renderpass command_buffer::recording::begin_renderpass(std::weak_ptr<renderpass> rp, VkFramebuffer framebuffer,  const std::vector<VkClearValue>& clear_values, VkRect2D render_area) noexcept
            {
                return record_renderpass(*this, rp, framebuffer, clear_values, render_area);
            }

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

            void command_buffer::recording::record_renderpass::bind_vertex_buffers(VkBuffer* buffers) const noexcept
            {
                // logger::info("Binding vertex buffer...");
                u32 first_binding { 0 };
                u32 binding_count { 1 };
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(_recording._buffer.handle(), first_binding, binding_count, buffers, offsets);
            }

            void command_buffer::recording::record_renderpass::bind_index_buffers(VkBuffer buffer, VkDeviceSize size) const noexcept
            {
                VkDeviceSize offset = 0;
                vkCmdBindIndexBuffer(_recording._buffer.handle(), buffer, offset, VK_INDEX_TYPE_UINT16);
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

            void command_buffer::recording::record_renderpass::draw_indexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) const noexcept
            {
                vkCmdDrawIndexed(_recording._buffer.handle(), index_count, instance_count, first_index, vertex_offset, first_instance);
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
            
            ////////////////////////////////////////////////
            ///              BEGIN TRANSFER              ///
            ////////////////////////////////////////////////
            command_buffer::recording::record_transfer::record_transfer(recording& rec) noexcept
                : _recording{ rec }
            {}

            command_buffer::recording::record_transfer command_buffer::recording::begin_transfer() noexcept
            {
                return record_transfer(*this);
            }

            void command_buffer::recording::record_transfer::copy_buffers(VkBuffer src, VkBuffer dst, const VkDeviceSize size) const noexcept
            {
                constexpr u32 region_count { 1 };
                VkBufferCopy copy_region {
                    .srcOffset = 0,
                    .dstOffset = 0,
                    .size = size
                };

                vkCmdCopyBuffer(_recording._buffer.handle(), src, dst, region_count, &copy_region);
            }

            bool command_buffer::recording::record_transfer::end() noexcept
            {
                return false;
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace
