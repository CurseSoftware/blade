#include "gfx/vulkan/types.h"
#include "gfx/vulkan/utils.h"
#include "gfx/vulkan/common.h"

#include <cstring>
#include <map>
#include <optional>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            std::optional<device> device::create(const struct instance& instance) noexcept
            {
                device device {};
                device.allocator = instance.allocator;
                device.physical_device = pick_physical_device(instance);
                
                if (device.physical_device == VK_NULL_HANDLE)
                {
                    logger::error("Failed to create vulkan device.");
                    return std::nullopt;
                }

                auto graphics_queue_opt = device.find_graphics_queue();
                if (!graphics_queue_opt.has_value())
                {
                    logger::error("Failed to create vulkan device: could not find graphics queue family");
                    return std::nullopt;
                }

                device.graphics_queue_family = graphics_queue_opt.value();

                device.create_logical_device(instance);

                return device;
            }

            void device::create_logical_device(const instance& instance) noexcept
            {
                const f32 graphics_queue_priority = 1.0f;
                VkPhysicalDeviceFeatures default_physical_device_features {};
                
                VkDeviceQueueCreateInfo graphics_queue_info = {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .flags = 0,
                    .queueFamilyIndex = graphics_queue_family.index,
                    .queueCount = device::GRAPHICS_QUEUE_COUNT,
                    .pQueuePriorities = &graphics_queue_priority
                };
                
                VkDeviceCreateInfo create_info = {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                    .queueCreateInfoCount = 1,
                    .pQueueCreateInfos = &graphics_queue_info,
                    .pEnabledFeatures = &default_physical_device_features,
                };

                VK_ASSERT(vkCreateDevice(physical_device, &create_info, nullptr, &logical_device));

                const u32 queue_index = 0;
                vkGetDeviceQueue(logical_device, graphics_queue_family.index, queue_index, &graphics_queue_family.queue);
            }

            void device::destroy() noexcept
            {
                logger::info("Destroying vulkan device...");
                vkDestroyDevice(logical_device, allocator);
                logger::info("Destroyed.");
            }

            std::optional<queue_family> device::find_graphics_queue() noexcept
            {
                
                queue_family queue {};
                auto index_opt = find_queue_family_index(VK_QUEUE_GRAPHICS_BIT);
                if (!index_opt.has_value())
                {
                    logger::error("Failed to find queue family with graphics bit");
                    return std::nullopt;
                }

                return queue;
            }

            std::optional<u32> device::find_queue_family_index(VkQueueFlagBits queue_type) noexcept
            {
                u32 queue_family_count = 0;
                VkQueueFamilyProperties* dummy_queue_family_properties = nullptr;
                std::vector<VkQueueFamilyProperties> queue_families {};
                
                vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, dummy_queue_family_properties);
                queue_families.resize(queue_family_count);
                vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

                usize i = 0;
                for (const auto& queue_family : queue_families)
                {
                    if (queue_family.queueFlags & queue_type)
                    {
                        return static_cast<u32>(i);
                    }

                    i++;
                }

                return std::nullopt;
            }

            VkPhysicalDevice device::pick_physical_device(const struct instance &instance) noexcept
            {
                VkPhysicalDevice physical_device = VK_NULL_HANDLE;
                u32 device_count = 0;
                VkPhysicalDevice* dummy_physical_devices = nullptr;
                std::vector<VkPhysicalDevice> physical_devices {};
                
                vkEnumeratePhysicalDevices(instance.instance, &device_count, dummy_physical_devices);

                if (device_count == 0)
                {
                    logger::error("Failed to find GPU's with Vulkan support.");
                    return VK_NULL_HANDLE;
                }

                physical_devices.resize(device_count);
                vkEnumeratePhysicalDevices(instance.instance, &device_count, physical_devices.data());

                // Thanks https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families
                std::multimap<i32, VkPhysicalDevice> candidates {};
                for (const auto& device : physical_devices)
                {
                    usize score = rate_physical_device(device);
                    candidates.insert(std::make_pair(score, device));
                }

                auto best_device_iter = candidates.rbegin();
                if (best_device_iter->first > 0)
                {
                    physical_device = best_device_iter->second;
                } 
                else
                {
                    logger::error("Failed to find suitable GPU");
                    return VK_NULL_HANDLE;
                }

                if (physical_device == VK_NULL_HANDLE)
                {
                    logger::error("Failed to find suitable GPU");
                    return VK_NULL_HANDLE;
                }

                return physical_device;
            }

            usize device::rate_physical_device(VkPhysicalDevice physical_device) noexcept
            {
                usize score = 0;
                VkPhysicalDeviceProperties properties {};
                VkPhysicalDeviceFeatures features {};
                
                vkGetPhysicalDeviceProperties(physical_device, &properties);
                vkGetPhysicalDeviceFeatures(physical_device, &features);

                if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                {
                    score += 1000;
                }

                score += properties.limits.maxImageDimension2D;

                if (!features.geometryShader)
                {
                    return 0;
                }

                logger::debug("Device {} got score {}", properties.deviceName, score);
                return score;
            }

            bool device::is_device_suitable(VkPhysicalDevice physical_device) noexcept
            {
                VkPhysicalDeviceProperties2 properties {};
                VkPhysicalDeviceFeatures2 features {};
                
                vkGetPhysicalDeviceProperties2(physical_device, &properties);
                vkGetPhysicalDeviceFeatures2(physical_device, &features);

                return properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
                                                        && features.features.geometryShader;;
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace
