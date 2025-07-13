#ifndef BLADE_GFX_VULKAN_TYPES_H
#define BLADE_GFX_VULKAN_TYPES_H

#include "gfx/vulkan/common.h"
#include <optional>

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
        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_TYPES_H
