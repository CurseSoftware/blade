#include "gfx/vulkan/platform.h"
#include "gfx/view.h"
#include "gfx/vulkan/common.h"

namespace blade 
{
    namespace gfx 
    {
        namespace vk
        {

            namespace platform
            {
                void set_surface_info(VkSurfaceCreateInfo &create_info_ref, struct framebuffer_create_info::native_window_data window_data)
                {
#ifdef BLADE_PLATFORM_LINUX
                    create_info_ref.dpy = window_data.display;
                    create_info_ref.window = window_data.window;
#elif defined(BLADE_PLATFORM_WINDOWS)
                    create_info_ref.hwnd = window_data.hwnd;
                    create_info_ref.hinstance = window_data.hinstance;
#endif
                }
            } // platform namespace

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
