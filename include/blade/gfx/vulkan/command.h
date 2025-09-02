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
                    [[nodiscard]] explicit command_buffer(VkCommandBuffer buffer) noexcept;

                    VkCommandBuffer handle() const noexcept { return _buffer; }

                    class recording;
                    std::optional<command_buffer::recording> begin(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) noexcept;
                    void reset() const noexcept;
                    void submit(const std::vector<VkSemaphore>& semaphores) const noexcept;
                    void end() noexcept;
                    [[nodiscard]] bool is_active() const noexcept { return _is_active; }

                public:
                    class recording 
                    {
                        public:
                            [[nodiscard]] explicit recording(command_buffer& cb) noexcept;

                            ~recording() noexcept;

                            recording(const recording&) = delete;
                            recording& operator=(const recording&) = delete;

                            recording(recording&& other) noexcept;

                            recording& operator=(recording&& other) noexcept;

                            class record_renderpass
                            {
                                public:
                                    record_renderpass(
                                        recording& rec
                                        , std::weak_ptr<renderpass> rp
                                        , VkFramebuffer framebuffer
                                        , const std::vector<VkClearValue>& clear_values
                                        , VkRect2D render_area
                                    ) noexcept;
                                    
                                    ~record_renderpass() noexcept;

                                    record_renderpass(const record_renderpass&) = delete;
                                    record_renderpass& operator=(const record_renderpass&) = delete;

                                    record_renderpass& operator=(record_renderpass&&) = delete;
                                    record_renderpass(record_renderpass&& other) noexcept;

                                    void bind_vertex_buffers(VkBuffer* buffers) const noexcept;
                                    void bind_pipeline(VkPipelineBindPoint bind_point, VkPipeline pipeline) const noexcept;
                                    void set_viewport(VkViewport viewport) const noexcept;
                                    void set_scissor(VkRect2D scissor) const noexcept;
                                    void draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) const noexcept;
                                    bool end() noexcept;
                                private:
                                    recording& _recording;
                                    std::weak_ptr<renderpass> _renderpass {};
                                    bool _active                          { false };
                            };

                            class record_transfer
                            {
                                public:
                                    [[nodiscard]] record_transfer(recording& rec) noexcept;

                                    void copy_buffers(VkBuffer scr, VkBuffer dst, const VkDeviceSize size) const noexcept;
                                    bool end() noexcept;
                                private:
                                    recording& _recording;
                            };

                            static std::optional<recording> create(command_buffer& cb, VkCommandBufferBeginInfo begin_info) noexcept;

                            [[nodiscard]] record_renderpass begin_renderpass(std::weak_ptr<renderpass> rp, VkFramebuffer framebuffer, const std::vector<VkClearValue>& clear_values, VkRect2D render_area) noexcept;
                            [[nodiscard]] record_transfer begin_transfer() noexcept;

                        private:
                            command_buffer& _buffer;
                    };
                
                private:
                    VkCommandBuffer _buffer { VK_NULL_HANDLE };
                    bool is_recording       { false };
                    bool _is_active         { false };
            };

            class command_pool
            {
                public:
                    [[nodiscard]] explicit command_pool() noexcept;
                    [[nodiscard]] explicit command_pool(
                        VkCommandPool pool
                        , std::weak_ptr<class device> device
                        , const VkAllocationCallbacks* callbacks
                    ) noexcept;

                    struct buffer_node
                    {
                        VkCommandBuffer command_buffer { VK_NULL_HANDLE };
                        VkFence fence                  { VK_NULL_HANDLE };
                        u64 last_frame_used            { 0 };
                        u64 creation_frame             { 0 };
                        u64 usage_count                { 0 };
                        buffer_node* next              { nullptr };

                        [[nodiscard]] explicit buffer_node(
                            VkCommandBuffer buffer
                            , u64 frame
                            , VkFence fence = VK_NULL_HANDLE
                        ) noexcept  
                            : command_buffer{ buffer }
                            , fence{ fence }
                            , last_frame_used{ frame }
                            , creation_frame{ frame }
                        {}
                    };

                    command_buffer& get_buffer(u32 index) const noexcept { return *_buffer_handlers[index].get(); }

                    /**
                     * @brief Create and allocate the command buffers from the command pool
                     * @param num_buffers The number of buffers to allocate
                     * @return `true` on success. `false` otherwise (typically attempting to create buffers after creating some before)
                     */
                    bool allocate_buffers(const u32 num_buffers) noexcept;

                    VkCommandBuffer acquire_command_buffer() noexcept;

                    std::optional<command_buffer> allocate_single() noexcept;

                    void destroy() noexcept;

                    [[nodiscard]] VkCommandPool handle() const noexcept { return _command_pool; }

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

                private:
                    std::optional<VkFence> create_fence_() const noexcept;

                private:
                    std::weak_ptr<class device> _device                           {};
                    VkCommandPool _command_pool                                   { VK_NULL_HANDLE };
                    const VkAllocationCallbacks* _allocation_callbacks            { nullptr };
                    std::vector<VkCommandBuffer> _buffers                         {};
                    std::vector<std::unique_ptr<command_buffer>> _buffer_handlers {};

                    buffer_node* _free_list_head { nullptr };
            };
        } // vk namespace
    } // gfx namespace 
} // blade namespace

#endif // BLADE_GFX_VULKAN_COMMAND_POOL_H
