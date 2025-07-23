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
        } // vk namespace
    } // gfx namespace
} // blade namespace
