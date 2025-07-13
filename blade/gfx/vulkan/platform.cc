#include "gfx/vulkan/platform.h"
#include "gfx/vulkan/common.h"

namespace blade 
{
    namespace gfx 
    {
        namespace vk
        {
            void get_platform_extensions(std::vector<const char*>& extensions)
            {
                #if defined(BLADE_PLATFORM_WINDOWS)
                extensions.push_back("VK_KHR_Win32_surface");
                #elif defined(BLADE_PLATFORM_LINUX)
                // NOTE: if using xcb, use xcb rather than xlib
                extensions.push_back("VK_KHR_xlib_surface");
                #endif
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace
