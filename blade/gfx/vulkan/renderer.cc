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

                auto device_opt = device::create(_instance);
                device = device_opt.value();

                _is_initialized = true;
                return true;
            }

            bool vulkan_backend::shutdown() noexcept
            {
                logger::info("Vulkan backend shutting down");

                for (auto view : _views)
                {
                    logger::info("Destroying view: {}", view.first.index);
                    view.second.destroy();
                    logger::info("Destroyed view {}.", view.first.index);
                }

                device.destroy();

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

                const framebuffer_handle handle {.index=framebuffer_handle_id};
                _views.insert(std::make_pair(handle, view));

                framebuffer_handle_id += 1;
                
                logger::info("Created framebuffer.");
                return handle;
            }

            void vulkan_backend::frame() noexcept
            {
            }

            void vulkan_backend::view::destroy()
            {
                surface.destroy();
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace
