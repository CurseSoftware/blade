#ifndef BLADE_GFX_VULKAN_COMMON_H
#define BLADE_GFX_VULKAN_COMMON_H

#include "core/core.h"

#ifdef BLADE_PLATFORM_LINUX
    #define VK_USE_PLATFORM_XLIB_KHR
    #define VK_NO_PLATFORM_XCB_KHR
#elif defined(BLADE_PLATFORM_WINDOWS)
    #define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>

#endif // BLADE_GFX_VULKAN_COMMON_H
