#ifndef BLADE_GFX_VULKAN_COMMAND_POOL_H
#define BLADE_GFX_VULKAN_COMMAND_POOL_H
#include "gfx/vulkan/common.h"
#include "gfx/vulkan/device.h"
#include "gfx/vulkan/renderpass.h"
#include <optional>
#include <limits>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            class command_buffer
            {
                public:
                    [[nodiscard]] explicit command_buffer(VkCommandBuffer buffer) noexcept
                        : _buffer { buffer }
                    {}

                    VkCommandBuffer handle() const noexcept { return _buffer; }

                    class recording;
                    std::optional<command_buffer::recording> begin(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) noexcept;

                public:
                    class recording 
                    {
                        public:
                            recording(command_buffer& cb)
                                 : _buffer{ cb }
                            {}

                            ~recording() noexcept
                            {
                                _buffer.is_recording = false;
                            }

                            recording(const recording&) = delete;
                            recording& operator=(const recording&) = delete;

                            recording(recording&& other) noexcept
                                : _buffer{ other._buffer }
                            {}

                            recording& operator=(recording&& other) noexcept
                            {
                                if (this != &other)
                                {
                                    _buffer = other._buffer;
                                }

                                return *this;
                            }

                            class record_renderpass
                            {
                                public:
                                    record_renderpass(
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

                                    ~record_renderpass() noexcept
                                    {
                                        if (_active)
                                        {
                                            logger::warn("Renderpass Recording destroying without being submitted");
                                        }
                                    }

                                    record_renderpass(const record_renderpass&) = delete;
                                    record_renderpass& operator=(const record_renderpass&) = delete;

                                    record_renderpass& operator=(record_renderpass&&) = delete;
                                    record_renderpass(record_renderpass&& other) noexcept
                                        : _recording{ other._recording }
                                        , _renderpass{ other._renderpass }
                                        , _active{ other._active }
                                    {
                                    }

                                    void bind_pipeline(VkPipelineBindPoint bind_point, VkPipeline pipeline) const noexcept;
                                    void set_viewport(VkViewport viewport) const noexcept;
                                    void set_scissor(VkRect2D scissor) const noexcept;
                                    void draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) const noexcept;
                                    bool end() const noexcept;
                                private:
                                    recording& _recording;
                                    std::weak_ptr<renderpass> _renderpass {};
                                    bool _active                          { false };
                            };

                            static std::optional<recording> create(command_buffer& cb, VkCommandBufferBeginInfo begin_info) noexcept;

                            record_renderpass begin_renderpass(recording& rec, std::weak_ptr<renderpass> rp, VkFramebuffer framebuffer, const std::vector<VkClearValue>& clear_values, VkRect2D render_area) noexcept;

                        private:
                            command_buffer& _buffer;
                    };
                
                private:
                    VkCommandBuffer _buffer { VK_NULL_HANDLE };
                    bool is_recording       { false };
            };

            class command_pool
            {
                public:
                    [[nodiscard]] explicit command_pool(
                        VkCommandPool pool
                        , std::weak_ptr<class device> device
                        , const VkAllocationCallbacks* callbacks
                    ) noexcept
                        : _command_pool{ pool }
                        , _device{ device }
                        , _allocation_callbacks{ callbacks }
                    {}

                    /**
                     * @brief Create and allocate the command buffers from the command pool
                     * @param num_buffers The number of buffers to allocate
                     * @return `true` on success. `false` otherwise (typically attempting to create buffers after creating some before)
                     */
                    bool allocate_buffers(const u32 num_buffers) noexcept;

                    void destroy() noexcept;

                public:
                    enum kind
                    {
                        transient,
                        reset
                    };

                    struct builder
                    {
                        [[nodiscard]] explicit builder(std::weak_ptr<class device> device) noexcept 
                            : info { device }
                        {}

                        std::optional<std::shared_ptr<command_pool>> build() const noexcept;

                        builder& set_queue_family_index(const u32 index) noexcept;
                        builder& use_allocation_callbacks(const VkAllocationCallbacks* callbacks) noexcept;

                        struct
                        {
                            std::weak_ptr<class device> device                  {};
                            enum kind kind                                      { kind::reset };
                            const VkAllocationCallbacks* allocation_callbacks   { nullptr };
                            u32 queue_family_index                              { std::numeric_limits<u32>::max() };
                        } info {};
                    };

                    [[nodiscard]] explicit command_pool() noexcept;
                private:
                    std::weak_ptr<class device> _device                           {};
                    VkCommandPool _command_pool                                   { VK_NULL_HANDLE };
                    const VkAllocationCallbacks* _allocation_callbacks            { nullptr };
                    std::vector<VkCommandBuffer> _buffers                         {};
                    std::vector<std::unique_ptr<command_buffer>> _buffer_handlers {};
            };
        } // vk namespace
    } // gfx namespace 
} // blade namespace

#endif // BLADE_GFX_VULKAN_COMMAND_POOL_H
