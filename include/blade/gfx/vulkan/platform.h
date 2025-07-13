#ifndef BLADE_GFX_VULKAN_PLATFORM_H
#define BLADE_GFX_VULKAN_PLATFORM_H

#include <vector>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            /// @brief Append platform-specific vulkan extensions to the list
            /// @param extensions Vector of extensions to append to
            void get_platform_extensions(std::vector<const char*>& extensions);
        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_PLATFORM_H
