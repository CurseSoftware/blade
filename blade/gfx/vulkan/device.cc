// #include "gfx/vulkan/types.h"
#include "gfx/vulkan/instance.h"
#include "gfx/vulkan/device.h"
#include "gfx/vulkan/surface.h"
#include "gfx/vulkan/utils.h"
#include "gfx/vulkan/common.h"

#include <cstring>
#include <map>
#include <optional>
#include <set>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            std::optional<std::shared_ptr<device>> device::builder::build() const noexcept
            {
                std::vector<std::shared_ptr<physical_device>> valid_devices = find_valid_devices_();
                if (valid_devices.empty())
                {
                    logger::error("No valid devices from specified requirements");
                    return std::nullopt;
                }

                // TODO: better device selection of remaining ones
                // physical_device selected_physical_device = std::move(valid_devices[0]);
                auto device = std::make_shared<class device>(valid_devices[0]);
                // device device (std::move(valid_devices[0]));
                std::vector<VkDeviceQueueCreateInfo> queue_infos = device->_physical_device->get_queue_family_infos();
                
                VkDeviceCreateInfo create_info {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                    .queueCreateInfoCount = static_cast<u32>(queue_infos.size()),
                    .pQueueCreateInfos = queue_infos.data(),
                    .enabledExtensionCount = static_cast<u32>(info.required_extensions.size()),
                    .ppEnabledExtensionNames = info.required_extensions.data(),
                    .pEnabledFeatures = device->_physical_device->get_features_ptr()
                };

                VkResult result = vkCreateDevice(
                    device->_physical_device->handle()
                    , &create_info
                    , info.allocation_callbacks
                    , &device->_logical_device
                );

                if (result != VK_SUCCESS)
                {
                    return std::nullopt;
                }

                return std::move(device);
            }

            device::builder& device::builder::set_allocation_callbacks(VkAllocationCallbacks *callbacks) noexcept
            {
                info.allocation_callbacks = callbacks;
                return *this;
            }

            device::builder& device::builder::require_features(VkPhysicalDeviceFeatures features) noexcept
            {
                info.physical_device_features = features;
                return *this;
            }
            
            device::builder& device::builder::require_queue(queue_type type) noexcept
            {
                switch (type)
                {
                    case queue_type::graphics:
                        info.require_graphics_queue = true;
                    case queue_type::present:
                        info.require_present_queue = true;
                    case queue_type::compute:
                        info.require_compute_queue = true;
                    case queue_type::transfer:
                        info.require_transfer_queue = true;
                }
                return *this;
            }

            device::builder& device::builder::require_extension(const char* extension_name) noexcept
            {
                if (extension_name != nullptr)
                {
                    info.required_extensions.push_back(extension_name);
                }

                return *this;
            }

            std::vector<std::shared_ptr<physical_device>> device::builder::find_valid_devices_() const noexcept
            {
                auto vk_physical_devices = info.instance.lock()->enumerate_physical_devices();
                std::vector<std::shared_ptr<physical_device>> physical_devices {};
                for (const auto& device : vk_physical_devices)
                {
                    auto pd = std::make_shared<physical_device>(device);
                    physical_devices.push_back(pd);
                }

                std::erase_if(physical_devices, [this](std::shared_ptr<physical_device> device) -> bool {
                    constexpr bool should_erase = true;
                    constexpr bool should_not_erase = false;

                    logger::info("Checking device {}", device->name());

                    for (auto extension : info.required_extensions)
                    {
                        logger::info("Checking extension {}", extension);
                        if (!device->extension_is_supported(extension))
                        {
                            logger::info("Physical Device {} does not support required extension {}. Removing.", device->name(), extension);
                            return should_erase;
                        }
                        logger::info("Physical Device {} supports required extension {}", device->name(), extension);

                        if (info.require_graphics_queue && !device->graphics_queue_index().has_value())
                        {
                            logger::info("Physical Device {} does not have required graphics queue. Removing.", device->name());
                            return should_erase;
                        }
                        
                        if (info.require_transfer_queue && !device->transfer_queue_index().has_value())
                        {
                            logger::info("Physical Device {} does not have required transfer queue. Removing.", device->name());
                            return should_erase;
                        }
                        
                        if (info.require_compute_queue && !device->compute_queue_index().has_value())
                        {
                            logger::info("Physical Device {} does not have required compute queue. Removing.", device->name());
                            return should_erase;
                        }

                    }
                    
                    return should_not_erase;
                });
              
                return std::move(physical_devices);
            }

            std::optional<VkQueue> device::get_queue(const queue_type type) const noexcept
            {
                constexpr const u32 queue_index = 0;
                VkQueue queue {};
                auto index_opt = get_queue_index_(type);
                if (!index_opt.has_value())
                {
                    return std::nullopt;
                }
                u32 index = index_opt.value();

                vkGetDeviceQueue(
                    _logical_device
                    , index
                    , queue_index
                    , &queue
                );

                return queue;
            }

            std::optional<u32> device::get_queue_index_(const queue_type type) const noexcept
            {
                switch(type)
                {
                    case queue_type::graphics:
                        return _physical_device->graphics_queue_index();
                    case queue_type::compute:
                        return _physical_device->compute_queue_index();
                    case queue_type::transfer:
                        return _physical_device->transfer_queue_index();
                    default:
                        return std::nullopt;
                }

                return std::nullopt;
            }

            VkSurfaceCapabilitiesKHR device::surface_capabilities(const struct surface& surface) const noexcept
            {
                VkSurfaceCapabilitiesKHR capabilities {};
                auto handle = _physical_device->handle();

                // TODO: check error value here
                // potentially return std::optional from this
                (void) vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                    _physical_device->handle()
                    , surface.vk_surface
                    , &capabilities 
                );

                return capabilities;
            }

            const std::vector<VkSurfaceFormatKHR> device::surface_formats(const struct surface& surface) const noexcept
            {
                u32 format_count = 0;
                VkSurfaceFormatKHR* dummy_formats = nullptr;
                
                // TODO: check error value here
                // potentially return std::optional from this
                std::vector<VkSurfaceFormatKHR> formats {};
                (void) vkGetPhysicalDeviceSurfaceFormatsKHR(
                    _physical_device->handle()
                    , surface.vk_surface
                    , &format_count
                    , dummy_formats
                );

                formats.resize(format_count);

                (void) vkGetPhysicalDeviceSurfaceFormatsKHR(
                    _physical_device->handle()
                    , surface.vk_surface
                    , &format_count
                    , formats.data()
                );

                return formats;
            }
            
            const std::vector<VkPresentModeKHR> device::surface_present_modes(const struct surface& surface) const noexcept
            {
                u32 present_mode_count = 0;
                VkPresentModeKHR* dummy_present_modes = nullptr;
                std::vector<VkPresentModeKHR> present_modes {};

                vkGetPhysicalDeviceSurfacePresentModesKHR(
                    _physical_device->handle()
                    , surface.vk_surface
                    , &present_mode_count
                    , dummy_present_modes
                );

                present_modes.resize(present_mode_count);
                vkGetPhysicalDeviceSurfacePresentModesKHR(
                    _physical_device->handle()
                    , surface.vk_surface
                    , &present_mode_count
                    , present_modes.data() 
                );

                return present_modes;
            }
            
            void device::destroy() noexcept
            {
                vkDestroyDevice(_logical_device, _allocation_callbacks);
            }

//
//            std::optional<device> device::create(const struct instance& instance, create_options options) noexcept
//            {
//                device device {};
//                device.allocator = instance.allocation_callbacks();
//                std::vector<const char*> device_extensions {};
//
//                if (options.use_swapchain)
//                {
//                    device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
//                }
//                
//                device.physical_device = pick_physical_device(instance, device_extensions);
//                
//                if (device.physical_device == VK_NULL_HANDLE)
//                {
//                    logger::error("Failed to create vulkan device.");
//                    return std::nullopt;
//                }
//
//                auto graphics_queue_opt = device.find_graphics_queue();
//                if (!graphics_queue_opt.has_value())
//                {
//                    logger::error("Failed to create vulkan device: could not find graphics queue family");
//                    return std::nullopt;
//                }
//
//                device.graphics_queue_family = graphics_queue_opt.value();
//
//                device.create_logical_device(instance, device_extensions);
//
//                return device;
//            }
//
//            void device::create_logical_device(const instance& instance, const std::vector<const char*>& extensions) noexcept
//            {
//                const f32 queue_priority = 1.0f;
//                VkPhysicalDeviceFeatures default_physical_device_features {};
//                std::vector<VkDeviceQueueCreateInfo> queue_create_infos {};
//                std::set<u32> unique_queue_families = { graphics_queue_family.index, present_queue_family.index };
//                
//                for (const u32& queue_family : unique_queue_families)
//                {
//                    VkDeviceQueueCreateInfo queue_info = {
//                        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
//                        .flags = 0,
//                        .queueFamilyIndex = graphics_queue_family.index,
//                        .queueCount = 1,
//                        .pQueuePriorities = &queue_priority
//                    };
//                    queue_create_infos.push_back(queue_info);
//                }
//
//                
//                VkDeviceCreateInfo create_info = {
//                    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
//                    .queueCreateInfoCount = static_cast<u32>(queue_create_infos.size()),
//                    .pQueueCreateInfos = queue_create_infos.data(),
//                    .enabledExtensionCount = static_cast<u32>(extensions.size()),
//                    .ppEnabledExtensionNames = extensions.data(),
//                    .pEnabledFeatures = &default_physical_device_features,
//                };
//
//                VK_ASSERT(vkCreateDevice(physical_device, &create_info, nullptr, &logical_device));
//
//                const u32 queue_index = 0;
//                vkGetDeviceQueue(logical_device, graphics_queue_family.index, queue_index, &graphics_queue_family.queue);
//                vkGetDeviceQueue(logical_device, graphics_queue_family.index, queue_index, &present_queue_family.queue);
//            }
//
//
//            std::optional<queue_family> device::find_graphics_queue() noexcept
//            {
//                
//                queue_family queue {};
//                auto index_opt = find_queue_family_index(VK_QUEUE_GRAPHICS_BIT);
//                if (!index_opt.has_value())
//                {
//                    logger::error("Failed to find queue family with graphics bit");
//                    return std::nullopt;
//                }
//
//                return queue;
//            }
//
//            std::optional<queue_family> device::find_present_queue(const surface& surface) noexcept
//            {
//                queue_family queue {};
//                auto index_opt = device::find_present_queue_index(surface);
//                if (!index_opt.has_value())
//                {
//                    logger::error("Failed to find queue index with present bit");
//                    return std::nullopt;
//                }
//
//                return queue;
//            }
//
//            std::optional<u32> device::find_present_queue_index(const struct surface& surface) noexcept
//            {
//                u32 queue_family_count = 0;
//                VkQueueFamilyProperties* dummy_queue_family_properties = nullptr;
//                std::vector<VkQueueFamilyProperties> queue_families {};
//                
//                vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, dummy_queue_family_properties);
//                queue_families.resize(queue_family_count);
//                vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());
//
//                usize i = 0;
//                for (const auto& queue_family : queue_families)
//                {
//                    VkBool32 present_support = VK_FALSE;
//                    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface.vk_surface, &present_support);
//                    if (present_support)
//                    {
//                        return static_cast<u32>(i);
//                    }
//
//                    i++;
//                }
//
//                return std::nullopt;
//            }
//
//            std::optional<u32> device::find_queue_family_index(VkQueueFlagBits queue_type) noexcept
//            {
//                u32 queue_family_count = 0;
//                VkQueueFamilyProperties* dummy_queue_family_properties = nullptr;
//                std::vector<VkQueueFamilyProperties> queue_families {};
//                
//                vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, dummy_queue_family_properties);
//                queue_families.resize(queue_family_count);
//                vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());
//
//                usize i = 0;
//                for (const auto& queue_family : queue_families)
//                {
//                    if (queue_family.queueFlags & queue_type)
//                    {
//                        return static_cast<u32>(i);
//                    }
//
//                    i++;
//                }
//
//                return std::nullopt;
//            }
//
//            VkPhysicalDevice device::pick_physical_device(const struct instance &instance, const std::vector<const char*>& extensions) noexcept
//            {
//                VkPhysicalDevice physical_device = VK_NULL_HANDLE;
//                u32 device_count = 0;
//                VkPhysicalDevice* dummy_physical_devices = nullptr;
//                std::vector<VkPhysicalDevice> physical_devices {};
//                
//                vkEnumeratePhysicalDevices(instance.handle(), &device_count, dummy_physical_devices);
//
//                if (device_count == 0)
//                {
//                    logger::error("Failed to find GPU's with Vulkan support.");
//                    return VK_NULL_HANDLE;
//                }
//
//                physical_devices.resize(device_count);
//                vkEnumeratePhysicalDevices(instance.handle(), &device_count, physical_devices.data());
//
//                // Thanks https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families
//                std::multimap<i32, VkPhysicalDevice> candidates {};
//                for (const auto& device : physical_devices)
//                {
//                    usize score = rate_physical_device(device, extensions);
//                    candidates.insert(std::make_pair(score, device));
//                }
//
//                auto best_device_iter = candidates.rbegin();
//                if (best_device_iter->first > 0)
//                {
//                    physical_device = best_device_iter->second;
//                } 
//                else
//                {
//                    logger::error("Failed to find suitable GPU");
//                    return VK_NULL_HANDLE;
//                }
//
//                if (physical_device == VK_NULL_HANDLE)
//                {
//                    logger::error("Failed to find suitable GPU");
//                    return VK_NULL_HANDLE;
//                }
//
//                return physical_device;
//            }
//
//            usize device::rate_physical_device(VkPhysicalDevice physical_device, const std::vector<const char*>& extensions) noexcept
//            {
//                usize score = 0;
//                VkPhysicalDeviceProperties properties {};
//                VkPhysicalDeviceFeatures features {};
//
//                VkBool32 present_supported = VK_FALSE;
//                
//                vkGetPhysicalDeviceProperties(physical_device, &properties);
//                vkGetPhysicalDeviceFeatures(physical_device, &features);
//
//                if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
//                {
//                    score += 1000;
//                }
//
//                score += properties.limits.maxImageDimension2D;
//
//                if (!features.geometryShader)
//                {
//                    return 0;
//                }
//
//
//                bool supports_extensions = check_device_extension_support(physical_device, extensions);
//                if (!supports_extensions)
//                {
//                    return 0;
//                }
//
//                logger::debug("Device {} got score {}", properties.deviceName, score);
//                return score;
//            }
//
//            bool device::check_device_extension_support(VkPhysicalDevice physical_device, const std::vector<const char*>& required_device_extensions) noexcept
//            {
//                u32 extension_count = 0;
//                const char* layer_name = nullptr;
//                VkExtensionProperties *dummy_properties = nullptr;
//                std::vector<VkExtensionProperties> available_extensions {};
//
//                VK_ASSERT(vkEnumerateDeviceExtensionProperties(physical_device, layer_name, &extension_count, dummy_properties));
//                available_extensions.resize(extension_count);
//                
//                VK_ASSERT(vkEnumerateDeviceExtensionProperties(physical_device, layer_name, &extension_count, available_extensions.data()));
//
//                std::set<std::string> required_extensions(required_device_extensions.begin(), required_device_extensions.end());
//                
//                for (const auto& extension : available_extensions)
//                {
//                    required_extensions.erase(extension.extensionName);
//                }
//
//                return required_extensions.empty();
//            }
//
//            bool device::is_device_suitable(VkPhysicalDevice physical_device) noexcept
//            {
//                VkPhysicalDeviceProperties2 properties {};
//                VkPhysicalDeviceFeatures2 features {};
//                
//                vkGetPhysicalDeviceProperties2(physical_device, &properties);
//                vkGetPhysicalDeviceFeatures2(physical_device, &features);
//
//                return properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
//                                                        && features.features.geometryShader;;
//            }
//
//            struct swapchain::details device::query_swapchain_capabilities(const struct surface& surface) const noexcept
//            {
//                struct swapchain::details details {};
//                VkSurfaceFormatKHR* dummy_formats = nullptr;
//                VkPresentModeKHR* dummy_present_modes = nullptr;
//                u32 format_count = 0;
//                u32 present_mode_count = 0;
//
//                vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface.vk_surface, &details.capabilities);
//                vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface.vk_surface, &format_count, dummy_formats);
//                vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface.vk_surface, &format_count, dummy_present_modes);
//
//                if (format_count != 0)
//                {
//                    details.formats.resize(format_count);
//                    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface.vk_surface, &format_count, details.formats.data());
//                }
//
//                if (present_mode_count != 0)
//                {
//                    details.present_modes.resize(present_mode_count);
//                    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface.vk_surface, &format_count, details.present_modes.data());
//                }
//
//                return details;
//            }
        } // vk namespace
    } // gfx namespace
} // blade namespace
