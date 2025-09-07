#ifndef BLADE_GFX_VULKAN_VIEW_H
#define BLADE_GFX_VULKAN_VIEW_H

#include "gfx/program.h"
#include "gfx/view.h"
#include "gfx/vulkan/buffer.h"
#include "gfx/vulkan/command_handler.h"
#include "gfx/vulkan/common.h"
#include "gfx/vulkan/device.h"
#include "gfx/vulkan/renderpass.h"
#include "gfx/vulkan/swapchain.h"
#include "gfx/vulkan/instance.h"
#include "gfx/vulkan/pipeline.h"
#include "gfx/vulkan/command.h"

#include <memory>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            class view
            {
                public:
                    // TODO: use command submission from each view
                    view(std::weak_ptr<class device> device, std::shared_ptr<class surface> surface) noexcept;

                    void destroy() noexcept;
                    bool create_framebuffers() noexcept;

                    void set_viewport(f32 x, f32 y, struct width width, struct height height) noexcept;
                   
                    void record_commands(class command_buffer& command_buffer) const noexcept;

                    void frame() noexcept;

                    VkExtent2D get_extent() const noexcept;

                    static std::optional<view> create(std::weak_ptr<class instance> instance, std::weak_ptr<class device> device, const framebuffer_create_info info) noexcept;

                    bool create_program(const struct program& program, const shader& vertex, const shader& fragment) noexcept;

                    std::weak_ptr<class pipeline> get_graphics_pipeline() const noexcept { return graphics_pipeline; }

                    void attach_vertex_buffer(std::weak_ptr<buffer> buffer) noexcept;
                    
                    void set_vertex_buffer(std::weak_ptr<buffer> buffer) noexcept;
                    void set_index_buffer(std::shared_ptr<buffer> buffer) noexcept;


                private:
                    bool recreate_swapchain_(struct width width, struct height height) noexcept;
                    bool create_swapchain_(struct width width, struct height height) noexcept;
                    bool create_renderpass_() noexcept;
                    void destroy_framebuffers_() noexcept;
                    VkFormat get_format_() const noexcept;
                   
                private:
                    std::weak_ptr<class device> device                        {};
                    command_handler cmd_handler;
                    // std::shared_ptr<class command_pool> command_pool          {};
                    std::shared_ptr<struct surface> surface                   { nullptr };
                    std::optional<std::unique_ptr<class swapchain>> swapchain { std::nullopt };
                    std::vector<VkFramebuffer> framebuffers                   {};
                    VkAllocationCallbacks* allocation_callbacks               { nullptr };
                    std::unique_ptr<class pipeline::builder> pipeline_builder { nullptr };
                    std::shared_ptr<class pipeline> graphics_pipeline         { nullptr };
                    std::shared_ptr<class renderpass> renderpass              { nullptr };
                    struct program program                                    {};
                    VkViewport viewport                                       {};
                    VkSemaphore image_available_semaphore                     {};
                    VkSemaphore render_finished_semaphore                     {};
                    VkFence in_flight_fence                                   {};
                    u32 current_image_index                                   { 0 };
                    std::weak_ptr<class buffer> buffer                 {};
                    std::shared_ptr<class buffer> index_buffer                { nullptr };

                    u32 cached_width  { 0 };
                    u32 cached_height { 0 };
                    u32 cached_width_prev { 0 };
                    u32 cached_height_prev { 0 };
            };

        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_VIEW_H
