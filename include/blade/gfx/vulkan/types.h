#ifndef BLADE_GFX_VULKAN_TYPES_H
#define BLADE_GFX_VULKAN_TYPES_H

#include "gfx/vulkan/common.h"
#include "gfx/vulkan/instance.h"
#include "gfx/vulkan/device.h"
#include "gfx/vulkan/surface.h"
#include "gfx/vulkan/swapchain.h"
#include <functional>
#include <optional>
#include <utility>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
//            struct instance
//            {
//                /// @brief Create a vulkan instance
//                /// @return Some(instance) on success. `std::nullopt` otherwise
//                static std::optional<instance> create(VkAllocationCallbacks* allocator = nullptr) noexcept;
//
//                /// @brief Enable vulkan debug callbacks
//                /// @param allocator `VkAllocationCallbacks` allocator to allocate vulkan objects
//                void create_debug_messenger() noexcept;
//
//                void destroy() noexcept;
//
//                void destroy_debug_messenger() noexcept;
//
//                const std::vector<const char*> get_validation_layers() const noexcept { return validation_layers; }
//
//                VkAllocationCallbacks* allocator { nullptr };
//                VkInstance instance { VK_NULL_HANDLE };
//                VkDebugUtilsMessengerEXT debug_messenger { VK_NULL_HANDLE };
//                std::vector<const char*> validation_layers;
//            };

            struct queue_family
            {
                VkQueue queue { VK_NULL_HANDLE };
                u32 index { 0 };
            };
//            
//            struct swapchain
//            {
//                struct details 
//                {
//                    VkSurfaceCapabilitiesKHR capabilities {};
//                    std::vector<VkSurfaceFormatKHR> formats {};
//                    std::vector<VkPresentModeKHR> present_modes {};
//                } details;
//
//                struct create_info 
//                {
//                    enum present_mode present_mode { present_mode::FIFO };
//                    struct 
//                    {
//                        struct width width {0};
//                        struct height height {0};
//                    } extent;
//                    VkFormat preferred_format;
//                };
//               
//                // Constructors
//                [[nodiscard]] explicit swapchain(const struct device& device, const struct surface& surface) : device{device}, surface{surface} {}
//                
//                swapchain(swapchain&& other) noexcept 
//                    : device{other.device}
//                    , surface{other.surface}
//                    , vk_swapchain{std::exchange(other.vk_swapchain, VK_NULL_HANDLE)}
//                    , format{std::move(other.format)}
//                    , extent{std::move(other.extent)}
//                    , images{std::move(other.images)}
//                    , image_views{std::move(other.image_views)}
//                {}
//
//                swapchain(const swapchain& other) = delete;
//                void operator=(const swapchain& other) = delete;
//
//                swapchain& operator=(swapchain&& other) noexcept
//                {
//                    if (this != &other)
//                    {
//                        device = other.device;
//                        surface = other.surface;
//                        details = std::move(other.details);
//                        vk_swapchain = std::exchange(other.vk_swapchain, VK_NULL_HANDLE);
//                        format = std::move(other.format);
//                        extent = std::move(other.extent);
//                        images = std::move(other.images);
//                        image_views = std::move(other.image_views);
//                    }
//
//                    return *this;
//                }
//               
//                // Static Creation Methods
//                static std::optional<struct swapchain> create(const struct surface& surface, const struct device& device, create_info info) noexcept;
//                static VkSurfaceFormatKHR select_surface_format(VkFormat preferred_format, const std::vector<VkSurfaceFormatKHR>& formats) noexcept;
//                static VkPresentModeKHR select_present_mode(present_mode preferred_present_mode, const std::vector<VkPresentModeKHR>& available_present_modes) noexcept;
//                static VkExtent2D select_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, struct width width, struct height height) noexcept;
//
//                void create_image_views() noexcept;
//                void destroy() noexcept;
//
//                std::reference_wrapper<const struct device> device;
//                std::reference_wrapper<const struct surface> surface;
//                VkSwapchainKHR vk_swapchain { VK_NULL_HANDLE };
//                VkFormat format {};
//                VkExtent2D extent {};
//                std::vector<VkImage> images {};
//                std::vector<VkImageView> image_views {};
//            };

//            struct device
//            {
//                static const u32 GRAPHICS_QUEUE_COUNT = 1;
//
//                struct create_options 
//                {
//                    bool use_swapchain { false };
//                };
//
//                /// @brief Create a vk::device object to handle managing VkPhysicalDevice and VkDevice data
//                /// @param instance The vk::instance manager to create device from
//                static std::optional<device> create(const struct instance& instance, create_options options) noexcept;
//
//                /// @brief Rate the physical device for how useful it is
//                /// @return VkPhysicalDevice selected
//                static VkPhysicalDevice pick_physical_device(const struct instance& instance, const std::vector<const char*>& extensions) noexcept;
//
//                /// @brief Rate the physical device for how useful it is
//                /// @return score of the device. Higher is better
//                static usize rate_physical_device(VkPhysicalDevice physical_device, const std::vector<const char*>& extensions) noexcept;
//
//                /// @deprecated
//                /// @brief Determine if device is suitable. Only check if has discrete GPU
//                /// @return true if suitable false otherwise
//                static bool is_device_suitable(VkPhysicalDevice physical_device) noexcept;
//                
//                static bool check_device_extension_support(VkPhysicalDevice physical_device, const std::vector<const char*>& required_extensions) noexcept;
//
//                void create_logical_device(const instance& instance, const std::vector<const char*>& extensions) noexcept;
//
//                std::optional<queue_family> find_graphics_queue() noexcept;
//                std::optional<queue_family> find_present_queue(const struct surface& surface) noexcept;
//                std::optional<u32> find_present_queue_index(const struct surface& surface) noexcept;
//
//                /// @brief Find a queue family of a given type
//                /// @param queue_type VkQueueFlagBits of the queue to find
//                std::optional<u32> find_queue_family_index(VkQueueFlagBits queue_type) noexcept;
//
//                struct swapchain::details query_swapchain_capabilities(const struct surface& surface) const noexcept;
//
//                void destroy() noexcept;
//
//                VkAllocationCallbacks* allocator { nullptr };
//                VkPhysicalDevice physical_device { VK_NULL_HANDLE };
//                VkDevice logical_device { VK_NULL_HANDLE };
//                queue_family graphics_queue_family {};
//                queue_family present_queue_family {};
//                queue_family transfer_queue_family {};
//                VkCommandPool graphics_command_pool { VK_NULL_HANDLE };
//            };

        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_TYPES_H
