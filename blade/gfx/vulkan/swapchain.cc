#include "gfx/vulkan/types.h"
#include "gfx/vulkan/utils.h"
#include "gfx/vulkan/common.h"
#include <array>
#include <limits>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            std::optional<swapchain> swapchain::create(const struct surface& surface, const struct device& device, struct create_info create_info) noexcept
            {
                swapchain sw = swapchain(device, surface);
                auto details = device.query_swapchain_capabilities(surface);
                u32 num_image_array_layers = 1; // more than 1 layer per image is typically used in stereoscopic applications
                VkImage* dummy_images = nullptr;
                
                sw.details = details;
                
                VkSurfaceFormatKHR surface_format = select_surface_format(create_info.preferred_format, details.formats);
                VkPresentModeKHR present_mode = select_present_mode(create_info.present_mode, details.present_modes);
                VkExtent2D extent = select_swap_extent(details.capabilities, create_info.extent.width, create_info.extent.height);

                u32 image_count = details.capabilities.minImageCount + 1;
                logger::info("Min image count: {}", image_count);
                if (details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount)
                {
                    image_count = details.capabilities.maxImageCount;
                }

                VkSwapchainCreateInfoKHR swapchain_create_info {};
                swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
                swapchain_create_info.imageFormat = surface_format.format;
                swapchain_create_info.imageColorSpace = surface_format.colorSpace;
                swapchain_create_info.imageExtent = extent;
                swapchain_create_info.imageArrayLayers = num_image_array_layers;
                swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                swapchain_create_info.surface = surface.vk_surface;
                swapchain_create_info.minImageCount = image_count;


                if (device.graphics_queue_family.index != device.present_queue_family.index)
                {
                    std::array<u32, 2> queue_family_indices = { device.graphics_queue_family.index, device.present_queue_family.index };
                    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                    swapchain_create_info.queueFamilyIndexCount = 2;
                    swapchain_create_info.pQueueFamilyIndices = queue_family_indices.data();
                }
                else
                {
                    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                    swapchain_create_info.queueFamilyIndexCount = 0;
                    swapchain_create_info.pQueueFamilyIndices = nullptr;
                }

                swapchain_create_info.preTransform = details.capabilities.currentTransform;
                swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
                swapchain_create_info.presentMode = present_mode;
                swapchain_create_info.clipped = VK_TRUE;
                swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

                // VK_ASSERT(vkCreateSwapchainKHR(device.logical_device, &swapchain_create_info, nullptr, &sw.vk_swapchain));
                VK_ASSERT(vkCreateSwapchainKHR(device.logical_device, &swapchain_create_info, device.allocator, &sw.vk_swapchain));

                VK_ASSERT(vkGetSwapchainImagesKHR(device.logical_device, sw.vk_swapchain, &image_count, dummy_images));
                sw.images.resize(image_count);
                VK_ASSERT(vkGetSwapchainImagesKHR(device.logical_device, sw.vk_swapchain, &image_count, sw.images.data()));

                sw.format = surface_format.format;
                sw.extent = extent;
                sw.create_image_views();

                return sw;
            }

            void swapchain::create_image_views() noexcept
            {
                logger::info("Creating swapchain image views. {} images", images.size());
                image_views.resize(images.size());
                u32 base_mip_level = 0;
                u32 level_count = 1;
                u32 base_array_layer = 0;
                u32 layer_count = 1;

                for (usize i = 0; i < images.size(); i++)
                {
                    VkImageViewCreateInfo create_info {
                        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                        .image = images[i],
                        .viewType = VK_IMAGE_VIEW_TYPE_2D,
                        .format = format,
                    };

                    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

                    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    create_info.subresourceRange.baseMipLevel = base_mip_level;
                    create_info.subresourceRange.levelCount = level_count;
                    create_info.subresourceRange.baseArrayLayer = base_array_layer;
                    create_info.subresourceRange.layerCount = layer_count;

                    VK_ASSERT(vkCreateImageView(device.get().logical_device, &create_info, device.get().allocator, &image_views[i]));
                }
            }

            void swapchain::destroy() noexcept
            {
                logger::info("Destroying swapchain...");

                for (auto image_view : image_views)
                {
                    logger::info("Destroying image view...");
                    // vkDestroyImageView(device.get().logical_device, image_view, nullptr);
                    vkDestroyImageView(device.get().logical_device, image_view, device.get().allocator);
                    logger::info("Destroyed.");
                }
                // vkDestroySwapchainKHR(device.get().logical_device, vk_swapchain, nullptr);
                vkDestroySwapchainKHR(device.get().logical_device, vk_swapchain, device.get().allocator);
                logger::info("Destroyed.");
            }
            
            VkSurfaceFormatKHR swapchain::select_surface_format(VkFormat preferred_format, const std::vector<VkSurfaceFormatKHR>& available_formats) noexcept
            {
                VkSurfaceFormatKHR best_format = available_formats[0];
                for (const auto& available_format : available_formats)
                {
                    if (available_format.format == preferred_format)
                    {
                        best_format = available_format;
                    }

                    if (best_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    {
                        return best_format;
                    }
                }

                return best_format;
            }
                
            VkPresentModeKHR swapchain::select_present_mode(present_mode preferred_present_mode, const std::vector<VkPresentModeKHR>& available_present_modes) noexcept
            {
                VkPresentModeKHR preferred_present_mode_vk = present_mode_to_vulkan(preferred_present_mode);

                for (const auto& available_present_mode : available_present_modes)
                {
                    if (available_present_mode == preferred_present_mode_vk)
                        return available_present_mode;
                }

                return VK_PRESENT_MODE_FIFO_KHR;
            }

            VkExtent2D swapchain::select_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities, struct width width, struct height height) noexcept
            {
                if (capabilities.currentExtent.width != std::numeric_limits<u32>::max())
                {
                    return capabilities.currentExtent;
                }

                VkExtent2D extent = {
                    .width = std::clamp(width.w, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
                    .height = std::clamp(height.h, capabilities.minImageExtent.width, capabilities.maxImageExtent.height),
                };

                return extent;
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace
