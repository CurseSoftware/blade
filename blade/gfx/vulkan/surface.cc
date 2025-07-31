#include "gfx/vulkan/surface.h"
#include "gfx/vulkan/common.h"
#include "gfx/vulkan/platform.h"
#include "gfx/vulkan/utils.h"

namespace blade
{
    namespace gfx
    {
        namespace vk
        {

            std::optional<std::shared_ptr<surface>> surface::create(
                std::weak_ptr<const struct instance> instance,
                framebuffer_create_info create_info
            ) {
                logger::info("Creating vulkan surface.");
                auto surface = std::make_shared<struct surface>(instance);

                
                VkSurfaceCreateInfo surface_info {};
                surface_info.sType = VK_STRUCTURE_TYPE_SURFACE_CREATE_INFO_KHR;
                
                if (create_info.native_window_data.has_value())
                {
                    platform::set_surface_info(surface_info, create_info.native_window_data.value());
                    logger::info("Got platform extensions");
                }


                VK_ASSERT(vkCreateSurfaceKHR(
                    instance.lock()->handle()
                    , &surface_info
                    , instance.lock()->allocation_callbacks()
                    , &surface->vk_surface
                ));

                logger::info("Created.");

                return surface;
            }

            void surface::destroy()
            {
                logger::info("Destroying vulkan surface...");
                if (vk_surface != VK_NULL_HANDLE)
                {
                    vkDestroySurfaceKHR(instance.lock()->handle(), vk_surface, instance.lock()->allocation_callbacks());
                }
                logger::info("Destroyed.");
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace
