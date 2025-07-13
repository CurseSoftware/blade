#ifndef BLADE_GFX_VULKAN_TYPES_H
#define BLADE_GFX_VULKAN_TYPES_H

#include "gfx/vulkan/common.h"
#include <optional>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            struct instance
            {
                /// @brief Create a vulkan instance
                /// @return Some(instance) on success. `std::nullopt` otherwise
                static std::optional<instance> create(VkAllocationCallbacks* allocator = nullptr) noexcept;

                /// @brief Enable vulkan debug callbacks
                /// @param allocator `VkAllocationCallbacks` allocator to allocate vulkan objects
                void create_debug_messenger() noexcept;

                void destroy() noexcept;

                void destroy_debug_messenger() noexcept;

                VkAllocationCallbacks* allocator { nullptr };
                VkInstance instance { VK_NULL_HANDLE };
                VkDebugUtilsMessengerEXT debug_messenger { VK_NULL_HANDLE };
            };

            struct device
            {
                /// @brief Create a vk::device object to handle managing VkPhysicalDevice and VkDevice data
                /// @param instance The vk::instance manager to create device from
                static std::optional<device> create(const struct instance& instance) noexcept;

                static VkPhysicalDevice pick_physical_device(const struct instance& instance) noexcept;

                static usize rate_physical_device(VkPhysicalDevice physical_device) noexcept;

                static bool is_device_suitable(VkPhysicalDevice physical_device) noexcept;

                void destroy() noexcept;

                VkPhysicalDevice physical_device { VK_NULL_HANDLE };
                VkDevice logical_device { VK_NULL_HANDLE };
            };
        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_TYPES_H
