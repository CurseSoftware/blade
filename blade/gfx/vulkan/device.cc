#include "gfx/vulkan/types.h"
#include "gfx/vulkan/common.h"

#include <map>
#include <optional>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            std::optional<device> device::create(const struct instance& instance) noexcept
            {
                device device {};
                VkPhysicalDevice physical_device = pick_physical_device(instance);
                
                if (physical_device == VK_NULL_HANDLE)
                {
                    logger::error("Failed to create vulkan device.");
                    return std::nullopt;
                }

                device.physical_device = physical_device;

                return device;
            }

            void device::destroy() noexcept
            {
                // TODO
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
