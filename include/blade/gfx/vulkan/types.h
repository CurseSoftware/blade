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

                const std::vector<const char*> get_validation_layers() const noexcept { return validation_layers; }

                VkAllocationCallbacks* allocator { nullptr };
                VkInstance instance { VK_NULL_HANDLE };
                VkDebugUtilsMessengerEXT debug_messenger { VK_NULL_HANDLE };
                std::vector<const char*> validation_layers;
            };

            struct queue_family
            {
                VkQueue queue { VK_NULL_HANDLE };
                u32 index { 0 };
            };

            struct device
            {
                static const u32 GRAPHICS_QUEUE_COUNT = 1;

                /// @brief Create a vk::device object to handle managing VkPhysicalDevice and VkDevice data
                /// @param instance The vk::instance manager to create device from
                static std::optional<device> create(const struct instance& instance) noexcept;

                /// @brief Rate the physical device for how useful it is
                /// @return VkPhysicalDevice selected
                static VkPhysicalDevice pick_physical_device(const struct instance& instance) noexcept;

                /// @brief Rate the physical device for how useful it is
                /// @return score of the device. Higher is better
                static usize rate_physical_device(VkPhysicalDevice physical_device) noexcept;

                /// @deprecated
                /// @brief Determine if device is suitable. Only check if has discrete GPU
                /// @return true if suitable false otherwise
                static bool is_device_suitable(VkPhysicalDevice physical_device) noexcept;

                void create_logical_device(const instance& instance) noexcept;

                std::optional<queue_family> find_graphics_queue() noexcept;

                /// @brief Find a queue family of a given type
                /// @param queue_type VkQueueFlagBits of the queue to find
                std::optional<u32> find_queue_family_index(VkQueueFlagBits queue_type) noexcept;

                void destroy() noexcept;

                VkAllocationCallbacks* allocator { nullptr };
                VkPhysicalDevice physical_device { VK_NULL_HANDLE };
                VkDevice logical_device { VK_NULL_HANDLE };
                queue_family graphics_queue_family {};
                queue_family present_queue_family {};
                queue_family transfer_queue_family {};
                VkCommandPool graphics_command_pool { VK_NULL_HANDLE };
            };
        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_TYPES_H
