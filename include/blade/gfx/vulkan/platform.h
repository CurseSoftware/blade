#ifndef BLADE_GFX_VULKAN_PLATFORM_H
#define BLADE_GFX_VULKAN_PLATFORM_H
#include "gfx/vulkan/common.h"
#include "gfx/view.h"

#include <vector>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
#ifdef BLADE_PLATFORM_LINUX
            inline const auto& vkCreateSurfaceKHR = vkCreateXlibSurfaceKHR;
            inline const auto&  VK_STRUCTURE_TYPE_SURFACE_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
            using VkSurfaceCreateInfo = VkXlibSurfaceCreateInfoKHR;
#elif defined(BLADE_PLATFORM_WINDOWS)
            const auto& vkCreateSurfaceKHR = vkCreateWin32SurfaceKHR;
            const auto&  VK_STRUCTURE_TYPE_SURFACE_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            using VkSurfaceCreateInfo = VkWin32SurfaceCreateInfoKHR;
#endif

            namespace platform
            {
                /// @brief Create a vulkan surface from platform-specific info
                /// @param create_info The create info to set native-values to
                void set_surface_info(
                    VkSurfaceCreateInfo& create_info_ref,
                    struct framebuffer_create_info::native_window_data window_data
                );
            } // platform namespace
        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_PLATFORM_H
