#ifndef BLADE_GFX_VIEW_H
#define BLADE_GFX_VIEW_H

#include <optional>
#include "core/core.h"

#ifdef BLADE_PLATFORM_WINDOWS
#define NOMINMAX
#include <windows.h>
#elif defined(BLADE_PLATFORM_LINUX)
#include <X11/Xlib.h>
#endif

namespace blade
{
    namespace gfx
    {
        struct framebuffer_create_info
        {
            struct native_window_data
            {
#ifdef BLADE_PLATFORM_WINDOWS
                HWND hwnd;
                HINSTANCE hinstance;
#elif defined(BLADE_PLATFORM_LINUX)
                Display* display;
                Window window;
#endif
            };

            std::optional<native_window_data> native_window_data;
            struct width width{0};
            struct height height{0};
        };
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VIEW_H
