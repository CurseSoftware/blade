#ifndef BLADE_GFX_VULKAN_VIEW_H
#define BLADE_GFX_VULKAN_VIEW_H

#include "gfx/program.h"
#include "gfx/view.h"
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
            struct view
            {
                std::weak_ptr<class device> device                        {};
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

                void destroy() noexcept;
                bool create_framebuffers(std::weak_ptr<class renderpass> renderpass) noexcept;

                void set_viewport(f32 x, f32 y, struct width width, struct height height) noexcept;
               
                void record_commands(class command_buffer& command_buffer) const noexcept;

                void frame(class command_buffer& command_buffer) noexcept;

                VkExtent2D get_extent() const noexcept;

                static std::optional<view> create(std::weak_ptr<class instance> instance, std::weak_ptr<class device> device, const framebuffer_create_info info) noexcept;
            };

        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_VIEW_H
