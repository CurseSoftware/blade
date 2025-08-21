#include "gfx/vulkan/view.h"
#include "gfx/program.h"
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            view::view(std::weak_ptr<class device> device, std::shared_ptr<class surface> surface) noexcept
                : device{ device }
                , surface { surface }
                , pipeline_builder{ std::make_unique<pipeline::builder>(device) }
            {}
            
            std::optional<view> view::create(std::weak_ptr<class instance> instance, std::weak_ptr<class device> device, const framebuffer_create_info info) noexcept
            {
                auto surface_opt = surface::create(instance, info);
                if (!surface_opt.has_value())
                {
                    logger::error("Failed to create framebuffer");
                    return std::nullopt;
                }

                auto surface = surface_opt.value();
                class view view(device, surface);

                if (info.native_window_data)
                {
                    logger::info("Creating swapchain...");
                    view.create_swapchain_(info.width, info.height);

                    logger::info("Swapchain created.");
                }

                (void) view.create_renderpass_();
                view.pipeline_builder
                    // ->set_extent(view.get_extent())
                    ->add_viewport(VkViewport  {
                        .x = 0.0f,
                        .y = 0.0f,
                        .width =  static_cast<f32>(view.get_extent().width),
                        .height = static_cast<f32>(view.get_extent().height),
                        .minDepth = 0.0f,
                        .maxDepth = 1.0f
                    })
                    .add_scissor(VkRect2D {
                        .offset = { 0, 0 },
                        .extent = view.get_extent()
                    });

                VkSemaphoreCreateInfo semaphore_info { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
                VkFenceCreateInfo fence_info { 
                    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                    .flags = VK_FENCE_CREATE_SIGNALED_BIT
                };

                const VkResult image_available_sem_result = vkCreateSemaphore(device.lock()->handle(), &semaphore_info, nullptr, &view.image_available_semaphore);
                const VkResult render_finished_sem_result = vkCreateSemaphore(device.lock()->handle(), &semaphore_info, nullptr, &view.render_finished_semaphore);
                const VkResult fence_result = vkCreateFence(device.lock()->handle(), &fence_info, nullptr, &view.in_flight_fence);

                if (image_available_sem_result != VK_SUCCESS
                    || render_finished_sem_result != VK_SUCCESS
                    || fence_result != VK_SUCCESS
                ) {
                    logger::info("Failed to create synchronization objects");
                    return std::nullopt;
                }

                view.create_framebuffers();

                return view;
            }

            VkFormat view::get_format_() const noexcept
            {
                if (swapchain.has_value())
                {
                    return swapchain.value()->get_format();
                }

                // TODO: This is random and will 99.99% not work
                return VK_FORMAT_R8_SINT;
            }

            bool view::create_renderpass_() noexcept
            {
                VkAttachmentDescription color_attachment {
                    .format = get_format_(),
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                };
                
                VkAttachmentReference color_attachment_reference {
                    .attachment = 0,
                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                };

                VkSubpassDescription subpass {
                    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                    .colorAttachmentCount = 1,
                    .pColorAttachments = &color_attachment_reference,
                };
           
                VkSubpassDependency dependency {
                    .srcSubpass = VK_SUBPASS_EXTERNAL,
                    .dstSubpass = 0,
                    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                };

                renderpass = renderpass::builder(device)
                    .add_subpass_description(subpass)
                    .add_attachment(color_attachment)
                    .add_subpass_dependency(dependency)
                    .build()
                    .value();

                return true;
            }

            bool view::create_swapchain_(struct width width, struct height height) noexcept
            {
                swapchain = swapchain::builder(device, surface)
                    .set_allocation_callbacks(nullptr)
                    .set_composite_alpha(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
                    .require_image_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
                    .set_clipped(VK_TRUE)
                    .set_extent(width, height)
                    .prefer_present_mode(present_mode::MAILBOX)
                    .build();

                if (!swapchain.has_value())
                {
                    return false;
                }

                return true;
            }

            void view::set_viewport(f32 x, f32 y, struct width width, struct height height) noexcept
            {
                // logger::info("Viewport: {}, {}", width.w, height.h);
                viewport = VkViewport {
                    .x = x,
                    .y = y,
                    .width = static_cast<float>(width.w),
                    .height = static_cast<float>(height.h),
                    .minDepth = 0.f,
                    .maxDepth = 1.f
                };
            }

            bool view::create_program(const struct program& program, const shader& vertex, const shader& fragment) noexcept
            {
                this->program = program;
                graphics_pipeline = pipeline_builder
                    ->add_shader(shader::type::vertex, vertex.handle())
                    .add_shader(shader::type::fragment, fragment.handle())
                    .add_renderpass(renderpass->handle())
                    .add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT)
                    .add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR)
                    .build().value();

                return true;
            }
            
            bool view::create_framebuffers() noexcept
            {
                usize num_images = 1;
                if (swapchain.has_value())
                {
                    num_images = swapchain.value().get()->num_image_views();
                }

                for (usize i = 0; i < num_images; i++)
                {
                    logger::info("Creating Vulkan Framebuffer {}", i);
                    std::array<VkImageView, 1> attachments = {
                        swapchain.value().get()->get_image_view(i)
                    };

                    VkFramebufferCreateInfo framebuffer_info {
                        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                        .renderPass = renderpass->handle(),
                        .attachmentCount = static_cast<u32>(attachments.size()),
                        .pAttachments = attachments.data(),
                        .width = swapchain.value().get()->get_extent().width,
                        .height = swapchain.value().get()->get_extent().height,
                        .layers = 1
                    };
                    logger::info("Framebuffer info");

                    VkFramebuffer framebuffer;
                    const VkResult result = vkCreateFramebuffer(
                        device.lock()->handle(),
                        &framebuffer_info,
                        allocation_callbacks,
                        &framebuffer
                    );
                    framebuffers.push_back(framebuffer);

                    if (result != VK_SUCCESS)
                    {
                        return false;
                    }
                }

                return true;
            }

            void view::frame(class command_buffer& command_buffer) noexcept
            {
                
                const u32 fence_count = 1;
                const VkBool32 wait_all = VK_TRUE;
                const u64 timeout = UINT64_MAX;
                
                vkWaitForFences(device.lock()->handle(), fence_count, &in_flight_fence, wait_all, timeout);
                vkResetFences(device.lock()->handle(), fence_count, &in_flight_fence);
                
                if (swapchain.has_value())
                {
                    current_image_index = swapchain.value()->get_image_index(image_available_semaphore);
                }

                const std::vector<VkSemaphore> signal_semaphores = { render_finished_semaphore };
                const std::vector<VkSemaphore> wait_semaphores = { image_available_semaphore };
                
                command_buffer.reset();
                record_commands(command_buffer);
                std::array<VkCommandBuffer, 1> command_buffers = { command_buffer.handle() };
                
                VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
                
                VkSubmitInfo submit_info {
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .waitSemaphoreCount = static_cast<u32>(wait_semaphores.size()),
                    .pWaitSemaphores = wait_semaphores.data(),
                    .pWaitDstStageMask = wait_stages,
                    .commandBufferCount = 1,
                    .pCommandBuffers = command_buffers.data(),
                    .signalSemaphoreCount = static_cast<u32>(signal_semaphores.size()),
                    .pSignalSemaphores = signal_semaphores.data(),
                };

                const VkResult submit_result = vkQueueSubmit(device.lock()->get_queue(queue_type::graphics).value(), 1, &submit_info, in_flight_fence);
                if (submit_result != VK_SUCCESS)
                {
                    logger::error("FAILED TO SUBMIT");
                    return;
                }
                // command_buffer.submit(signal_semaphores);
                
                if (swapchain.has_value())
                {
                    std::array<VkSwapchainKHR, 1> swapchains = { swapchain.value()->handle() };
                    
                    VkPresentInfoKHR present_info {
                        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                        .waitSemaphoreCount = static_cast<u32>(signal_semaphores.size()),
                        .pWaitSemaphores = signal_semaphores.data(),
                        .swapchainCount = static_cast<u32>(swapchains.size()),
                        .pSwapchains = swapchains.data(),
                        .pImageIndices = &current_image_index
                    };

                    vkQueuePresentKHR(device.lock()->get_queue(queue_type::graphics).value(), &present_info);
                }
            }

            void view::destroy() noexcept
            {
                if (graphics_pipeline)
                {
                    logger::info("Destroying graphics pipeline...");
                    graphics_pipeline->destroy();
                    graphics_pipeline = nullptr;
                    logger::info("Destroyed.");
                }

                if (renderpass)
                {
                    logger::info("Destroying renderpass...");
                    renderpass->destroy();
                    renderpass = nullptr;
                    logger::info("Destroyed.");
                }

                vkDestroyFence(device.lock()->handle(), in_flight_fence, allocation_callbacks);
                vkDestroySemaphore(device.lock()->handle(), render_finished_semaphore, allocation_callbacks);
                vkDestroySemaphore(device.lock()->handle(), image_available_semaphore, allocation_callbacks);
                
                for (const auto& framebuffer : framebuffers)
                {
                    vkDestroyFramebuffer(device.lock()->handle(), framebuffer, allocation_callbacks);
                }
                
                if (swapchain.has_value())
                {
                    swapchain.value()->destroy();
                }
                surface->destroy();
            }

        } // vk namespace
    } // gfx namespace
} // blade namespace
