#include "gfx/vulkan/renderer.h"
#include "core/types.h"
#include "gfx/handle.h"
#include "gfx/view.h"
#include "gfx/vulkan/common.h"
#include "gfx/vulkan/platform.h"
#include "gfx/vulkan/types.h"
#include "gfx/vulkan/utils.h"
#include <cstring>
#include <optional>
#include <utility>
#include <vulkan/vulkan_core.h>

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

                std::vector<const char*> extensions = {};
                std::vector<const char*> validation_layers = {};

                if (init.require_surface) 
                {
                    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

                    auto platform_extensions = get_platform_extensions();
                    for (const auto& ext : platform_extensions)
                        extensions.push_back(ext);
                }

                // TODO: move to function
                if (init.enable_debug)
                {
                    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

                    validation_layers = get_debug_validation_layers().value();
                }

                auto instance_opt = instance::builder()
                    .set_engine_name("Blade Engine Name")
                    .set_application_name("Blade Application Name")
                    .set_engine_version(version { .major{1}, .minor{0}, .patch{0} })
                    .require_api_version(version { .major{1}, .minor{3}, .patch{0} })
                    .request_extensions(extensions)
                    .request_validation_layers(validation_layers)
                    .build();

                // auto instance_opt = instance::create();
                _instance = std::move(instance_opt.value());

                // _instance.create_debug_messenger();

                // auto device_opt = device::create(_instance, { .use_swapchain = true });
                // _device = device_opt.value();

                _is_initialized = true;
                return true;
            }

            std::vector<const char*> vulkan_backend::get_platform_extensions() const noexcept
            {
                std::vector<const char*> extensions {};
                #if defined(BLADE_PLATFORM_WINDOWS)
                extensions.push_back("VK_KHR_Win32_surface");
                #elif defined(BLADE_PLATFORM_LINUX)
                // NOTE: if using xcb, use xcb rather than xlib
                extensions.push_back("VK_KHR_xlib_surface");
                #endif

                return extensions;
            }

            std::optional<std::vector<const char*>> vulkan_backend::get_debug_validation_layers() const noexcept
            {
                std::vector<const char*> validation_layers { "VK_LAYER_KHRONOS_validation" };
                std::vector<VkLayerProperties> available_layers {};
                VkLayerProperties *dummy_layer_properties = nullptr;
                u32 available_layer_count = 0;
                

                VK_ASSERT(vkEnumerateInstanceLayerProperties(&available_layer_count, dummy_layer_properties));
                available_layers.resize(available_layer_count);
                VK_ASSERT(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers.data()));

                // TODO: move checking for validation layers into the instance::builder
                for (const auto& layer : validation_layers)
                {
                    bool found_layer = false;
                    logger::debug("Searching for validation layer {}", layer);

                    for (const auto& available_layer : available_layers)
                    {
                        if (strcmp(layer, available_layer.layerName) == 0)
                        {
                            found_layer = true;
                            logger::debug("Found.");
                            break;
                        }
                    }

                    if (!found_layer)
                    {
                        logger::error("Validation layer not found: {}", layer);
                        return std::nullopt;
                    }
                }

                return validation_layers;
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

                // _device.destroy();

                // _instance.destroy_debug_messenger();
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
