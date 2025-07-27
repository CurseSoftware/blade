#ifndef BLADE_GFX_VULKAN_SURFACE_H
#define BLADE_GFX_VULKAN_SURFACE_H
#include "gfx/vulkan/instance.h"
#include "gfx/view.h"
#include "gfx/vulkan/common.h"
#include <functional>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            struct surface
            {
                [[nodiscard]] explicit surface(const struct instance& instance) : instance{instance} {}

                surface(const surface& other) = delete;
                surface& operator=(const surface& other) = delete;

                surface(surface&& other) noexcept
                    : instance(std::move(other.instance))
                {
                    if (this != &other)
                    {
                        vk_surface = std::exchange(other.vk_surface, VK_NULL_HANDLE);
                    }
                }

                surface& operator=(surface&& other) noexcept
                {
                    if (this != &other)
                    {
                        vk_surface = std::exchange(other.vk_surface, VK_NULL_HANDLE);
                    }
                    
                    return *this;
                }
                
                static std::optional<surface> create(const struct instance& instance, framebuffer_create_info create_info);

                void destroy();

                std::reference_wrapper<const struct instance> instance;
                VkSurfaceKHR vk_surface { VK_NULL_HANDLE };
            };

        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_SURFACE_H
