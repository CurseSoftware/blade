#include "gfx/vulkan/renderer.h"
#include "gfx/handle.h"
#include "gfx/view.h"
#include <utility>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            vulkan_backend::vulkan_backend() noexcept
            {
                logger::debug("vulkan_backend constructor called");
            }

            vulkan_backend::~vulkan_backend()
            {
                logger::debug("vulkan_backend destructor called");
                if (_is_initialized)
                {
                    logger::warn("Vulkan backend destroying without explicit shutdown. Shutting down now.");
                    shutdown();
                }
            }

            bool vulkan_backend::init(const init_info& init) noexcept
            {
                logger::info("Initializing vulkan backend.");

                auto instance_opt = instance::create();
                _instance = instance_opt.value();

                _instance.create_debug_messenger();

                auto device_opt = device::create(_instance, { .use_swapchain = true });
                _device = device_opt.value();

                _is_initialized = true;
                return true;
            }

            bool vulkan_backend::shutdown() noexcept
            {
                logger::info("Vulkan backend shutting down");

                for (auto&& view : _views)
                {
                    logger::info("Destroying view: {}", view.first.index);
                    view.second.destroy();
                    logger::info("Destroyed view {}.", view.first.index);
                }

                _device.destroy();

                _instance.destroy_debug_messenger();
                _instance.destroy();

                _is_initialized = false;
                return true;
            }

            framebuffer_handle vulkan_backend::create_framebuffer(framebuffer_create_info create_info) noexcept
            {
                logger::info("Creating framebuffer...");
                static u16 framebuffer_handle_id = 0;
                
                auto surface_opt = surface::create(_instance, create_info);
                if (!surface_opt.has_value())
                {
                    logger::error("Failed to create framebuffer");
                    return framebuffer_handle{.index = BLADE_NULL_HANDLE};
                }

                auto surface = surface_opt.value();

                vulkan_backend::view view = {
                    .surface = surface
                };

                if (create_info.native_window_data)
                {
                    swapchain::create_info swapchain_create_info {};
                    swapchain_create_info.present_mode = present_mode::MAILBOX;
                    swapchain_create_info.preferred_format = VK_FORMAT_B8G8R8A8_SRGB;
                    swapchain_create_info.extent.width = create_info.width;
                    swapchain_create_info.extent.height = create_info.height;
                    
                    view.swapchain = swapchain::create(surface, _device, swapchain_create_info);
                    if (!view.swapchain.has_value())
                    {
                        logger::error("Failed to create swapchain");
                        return framebuffer_handle{.index = BLADE_NULL_HANDLE};
                    }
                    logger::info("Swapchain created.");
                }

                const framebuffer_handle handle {.index=framebuffer_handle_id};
                _views.insert(std::make_pair(handle, std::move(view)));

                framebuffer_handle_id += 1;
                
                logger::info("Created framebuffer.");
                return handle;
            }

            void vulkan_backend::frame() noexcept
            {
            }

            void vulkan_backend::view::destroy()
            {
                if (swapchain.has_value())
                {
                    swapchain.value().destroy();
                }
                surface.destroy();
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace
