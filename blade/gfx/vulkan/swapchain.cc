#include "gfx/vulkan/swapchain.h"
#include "gfx/vulkan/utils.h"
#include "gfx/vulkan/common.h"
#include <algorithm>
#include <array>
#include <limits>
#include <optional>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            std::optional<std::unique_ptr<swapchain>> swapchain::builder::build() const noexcept
            {
                auto swapchain = std::make_unique<class swapchain>(info.device);
                
                const u32 num_image_array_layers = 1;
                const VkSurfaceCapabilitiesKHR capabilities = info.device.lock()->surface_capabilities(*info.surface.lock());
                const auto formats = info.device.lock()->surface_formats(*info.surface.lock());
                const auto present_modes = info.device.lock()->surface_present_modes(*info.surface.lock());

                auto graphics_queue_index_opt = info.device.lock()->graphics_queue_index();
                auto present_queue_index_opt = info.device.lock()->present_queue_index(*info.surface.lock());

                if (!graphics_queue_index_opt.has_value())
                {
                    logger::error("Graphics queue does not have value");
                    return std::nullopt;
                }

                if (!present_queue_index_opt.has_value())
                {
                    logger::error("Present queue does not have value");
                    return std::nullopt;
                }
                
                u32 graphics_queue_index = graphics_queue_index_opt.value();
                u32 present_queue_index = present_queue_index_opt.value();
                const std::array<u32, 2> queue_family_indices = {graphics_queue_index, present_queue_index};

                VkSurfaceFormatKHR selected_format = select_surface_format_(formats);
                VkPresentModeKHR selected_present_mode = select_present_mode_(present_modes);
                VkExtent2D selected_extent = select_extent_(capabilities);

                const u32 image_count = [this, capabilities]() -> u32 
                {
                    switch(capabilities.maxImageCount)
                    {
                        // 0 Is special case with maxImageCount where there is "no limit" on swapchain images
                        case 0:
                            return std::max<u32>(info.min_image_count, capabilities.minImageCount+1);
                        
                        default:
                            return std::clamp<u32>(info.min_image_count, capabilities.minImageCount+1, capabilities.maxImageCount);
                    }
                }();


                if ((selected_present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR
                    || selected_present_mode == VK_PRESENT_MODE_MAILBOX_KHR
                    || selected_present_mode == VK_PRESENT_MODE_FIFO_KHR
                    || selected_present_mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR
                    ) && (
                        (info.image_usage & capabilities.supportedUsageFlags) != info.image_usage
                    )
                ) {
                    logger::error("Usage flags do not match presentation mode");
                    return std::nullopt;
                }

                VkSwapchainCreateInfoKHR create_info {
                    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                    .surface = info.surface.lock()->vk_surface,
                    .minImageCount = image_count,
                    .imageFormat = selected_format.format,
                    .imageColorSpace = selected_format.colorSpace,
                    .imageExtent = selected_extent,
                    .imageArrayLayers = num_image_array_layers,
                    .imageUsage = info.image_usage,
                };

                if (graphics_queue_index == present_queue_index)
                {
                    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                }
                else
                {
                    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                    create_info.queueFamilyIndexCount = 2;
                    create_info.pQueueFamilyIndices = queue_family_indices.data();
                }

                create_info.preTransform = capabilities.currentTransform;
                create_info.compositeAlpha = info.composite_alpha;
                create_info.presentMode = selected_present_mode;
                create_info.clipped = info.clipped;
                create_info.oldSwapchain = info.old_swapchain;

                const VkResult create_result = vkCreateSwapchainKHR(
                    info.device.lock()->handle()
                    , &create_info
                    , info.allocation_callbacks
                    , &swapchain->_swapchain
                );

                if (create_result != VK_SUCCESS)
                {
                    logger::error("vkCreateSwapchainKHR call failed");
                    return std::nullopt;
                }
                
                swapchain->_format = selected_format.format;
                swapchain->_extent = selected_extent;
                swapchain->_allocation_callbacks = info.allocation_callbacks;

                auto images_opt = swapchain->create_images_();
                if (!images_opt.has_value())
                {
                    logger::error("Failed to create images");
                    return std::nullopt;
                }
                swapchain->_images = images_opt.value();

                auto image_views_opt = swapchain->create_image_views_();
                if (!image_views_opt.has_value())
                {
                    logger::error("Failed to create image views");
                    return std::nullopt;
                }
                swapchain->_image_views = image_views_opt.value();

                return std::move(swapchain);
            }

            swapchain::builder& swapchain::builder::set_extent(struct width width, struct height height) noexcept
            {
                info.extent.width = width;
                info.extent.height = height;

                return *this;
            }

            swapchain::builder& swapchain::builder::prefer_present_mode(present_mode mode) noexcept
            {
                info.preferred_present_mode = mode;

                return *this;
            }

            swapchain::builder& swapchain::builder::request_min_image_count(u32 count) noexcept
            {
                info.min_image_count = count;

                return *this;
            }

            swapchain::builder& swapchain::builder::prefer_format(VkFormat format) noexcept
            {
                info.preferred_format = format;

                return *this;
            }

            swapchain::builder& swapchain::builder::require_image_usage(VkImageUsageFlagBits usage) noexcept
            {
                info.image_usage = usage;

                return *this;
            }
            
            swapchain::builder& swapchain::builder::set_composite_alpha(VkCompositeAlphaFlagBitsKHR alpha) noexcept
            {
                info.composite_alpha = alpha;

                return *this;
            }
            
            swapchain::builder& swapchain::builder::set_allocation_callbacks(VkAllocationCallbacks* callbacks) noexcept
            {
                info.allocation_callbacks = callbacks;

                return *this;
            }
            
            swapchain::builder& swapchain::builder::set_clipped(VkBool32 clipped) noexcept
            {
                info.clipped = clipped;

                return *this;
            }
            
            VkSurfaceFormatKHR swapchain::builder::select_surface_format_(const std::vector<VkSurfaceFormatKHR>& available_formats) const noexcept
            {
                VkSurfaceFormatKHR best_format = available_formats[0];
                for (const auto& available_format : available_formats)
                {
                    if (available_format.format == info.preferred_format)
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
                
            VkPresentModeKHR swapchain::builder::select_present_mode_(const std::vector<VkPresentModeKHR>& available_present_modes) const noexcept
            {
                VkPresentModeKHR preferred_present_mode_vk = present_mode_to_vulkan(info.preferred_present_mode);

                for (const auto& available_present_mode : available_present_modes)
                {
                    if (available_present_mode == preferred_present_mode_vk)
                        return available_present_mode;
                }

                return VK_PRESENT_MODE_FIFO_KHR;
            }

            VkExtent2D swapchain::builder::select_extent_(const VkSurfaceCapabilitiesKHR &capabilities) const noexcept
            {
                if (capabilities.currentExtent.width != std::numeric_limits<u32>::max())
                {
                    return capabilities.currentExtent;
                }

                VkExtent2D extent = {
                    .width = std::clamp(info.extent.width.w, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
                    .height = std::clamp(info.extent.height.h, capabilities.minImageExtent.width, capabilities.maxImageExtent.height),
                };

                return extent;
            }

            std::optional<std::vector<VkImage>> swapchain::create_images_() const noexcept
            {
                std::vector<VkImage> images {};
                u32 image_count = 0;
                VkImage* dummy_images = nullptr;

                VkResult result = vkGetSwapchainImagesKHR(
                    _device.lock()->handle()
                    , _swapchain
                    , &image_count
                    , dummy_images
                );

                if (result != VK_SUCCESS)
                {
                    return std::nullopt;
                }
                
                images.resize(image_count);
                
                result = vkGetSwapchainImagesKHR(
                    _device.lock()->handle()
                    , _swapchain
                    , &image_count
                    , images.data()
                );

                if (result != VK_SUCCESS)
                {
                    return std::nullopt;
                }


                return images;
            }


            //std::optional<swapchain> swapchain::create(const struct surface& surface, const struct device& device, struct create_info create_info) noexcept
            //{
//                swapchain sw = swapchain(device, surface);
//                auto details = device.query_swapchain_capabilities(surface);
//                u32 num_image_array_layers = 1; // more than 1 layer per image is typically used in stereoscopic applications
//                VkImage* dummy_images = nullptr;
//                
//                sw.details = details;
//                
//                VkSurfaceFormatKHR surface_format = select_surface_format(create_info.preferred_format, details.formats);
//                VkPresentModeKHR present_mode = select_present_mode(create_info.present_mode, details.present_modes);
//                VkExtent2D extent = select_swap_extent(details.capabilities, create_info.extent.width, create_info.extent.height);
//
//                u32 image_count = details.capabilities.minImageCount + 1;
//                logger::info("Min image count: {}", image_count);
//                if (details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount)
//                {
//                    image_count = details.capabilities.maxImageCount;
//                }
//
//                VkSwapchainCreateInfoKHR swapchain_create_info {};
//                swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
//                swapchain_create_info.imageFormat = surface_format.format;
//                swapchain_create_info.imageColorSpace = surface_format.colorSpace;
//                swapchain_create_info.imageExtent = extent;
//                swapchain_create_info.imageArrayLayers = num_image_array_layers;
//                swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
//                swapchain_create_info.surface = surface.vk_surface;
//                swapchain_create_info.minImageCount = image_count;
//
//
//                if (device.graphics_queue_family.index != device.present_queue_family.index)
//                {
//                    std::array<u32, 2> queue_family_indices = { device.graphics_queue_family.index, device.present_queue_family.index };
//                    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
//                    swapchain_create_info.queueFamilyIndexCount = 2;
//                    swapchain_create_info.pQueueFamilyIndices = queue_family_indices.data();
//                }
//                else
//                {
//                    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
//                    swapchain_create_info.queueFamilyIndexCount = 0;
//                    swapchain_create_info.pQueueFamilyIndices = nullptr;
//                }
//
//                swapchain_create_info.preTransform = details.capabilities.currentTransform;
//                swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
//                swapchain_create_info.presentMode = present_mode;
//                swapchain_create_info.clipped = VK_TRUE;
//                swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;
//
//                // VK_ASSERT(vkCreateSwapchainKHR(device.logical_device, &swapchain_create_info, nullptr, &sw.vk_swapchain));
//                VK_ASSERT(vkCreateSwapchainKHR(device.logical_device, &swapchain_create_info, device.allocator, &sw.vk_swapchain));
//
//                VK_ASSERT(vkGetSwapchainImagesKHR(device.logical_device, sw.vk_swapchain, &image_count, dummy_images));
//                sw.images.resize(image_count);
//                VK_ASSERT(vkGetSwapchainImagesKHR(device.logical_device, sw.vk_swapchain, &image_count, sw.images.data()));
//
//                sw.format = surface_format.format;
//                sw.extent = extent;
//                sw.create_image_views();
//
//                return std::nullopt;
//            }

            std::optional<std::vector<VkImageView>> swapchain::create_image_views_() const noexcept
            {
                if (_images.size() == 0)
                {
                    return std::nullopt;
                }
                
                std::vector<VkImageView> image_views {};
                image_views.resize(_images.size());
                u32 base_mip_level = 0;
                u32 level_count = 1;
                u32 base_array_layer = 0;
                u32 layer_count = 1;

                for (usize i = 0; i < _images.size(); i++)
                {
                    VkImageViewCreateInfo create_info {
                        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                        .image = _images[i],
                        .viewType = VK_IMAGE_VIEW_TYPE_2D,
                        .format = _format,
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

                    VkResult result = vkCreateImageView(
                        _device.lock()->handle()
                        , &create_info
                        , _allocation_callbacks
                        , &image_views[i]
                    );

                    if (result != VK_SUCCESS)
                    {
                        return std::nullopt;
                    }
                }
                
                return image_views;
            }

            void swapchain::destroy() noexcept
            {
                logger::info("Destroying swapchain...");

                for (auto image_view : _image_views)
                {
                    logger::info("Destroying image view...");
                    vkDestroyImageView(_device.lock()->handle(), image_view, _allocation_callbacks);
                    logger::info("Destroyed.");
                }
                vkDestroySwapchainKHR(_device.lock()->handle(), _swapchain, _allocation_callbacks);
                logger::info("Destroyed.");
            }
            
        } // vk namespace
    } // gfx namespace
} // blade namespace
