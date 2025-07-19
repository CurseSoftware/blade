#include "gfx/vulkan/types.h"
#include "gfx/vulkan/common.h"
#include "gfx/vulkan/platform.h"
#include "gfx/vulkan/utils.h"

namespace blade
{
    namespace gfx
    {
        namespace vk
        {

            std::optional<surface> surface::create(
                const struct instance& instance,
                framebuffer_create_info create_info
            ) {
                logger::info("Creating vulkan surface.");
                struct surface surface {instance};
                
                VkSurfaceCreateInfo surface_info {};
                surface_info.sType = VK_STRUCTURE_TYPE_SURFACE_CREATE_INFO_KHR;
                
                if (create_info.native_window_data.has_value())
                {
                    platform::set_surface_info(surface_info, create_info.native_window_data.value());
                }

                VK_ASSERT(vkCreateSurfaceKHR(
                    instance.instance
                    , &surface_info
                    , instance.allocator
                    , &surface.vk_surface
                ));

                return surface;
            }

            void surface::destroy()
            {
                logger::info("Destroying vulkan surface...");
                if (vk_surface != VK_NULL_HANDLE)
                {
                    vkDestroySurfaceKHR(instance.instance, vk_surface, instance.allocator);
                }
                logger::info("Destroyed.");
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace
