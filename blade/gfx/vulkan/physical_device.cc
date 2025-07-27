#include "gfx/vulkan/device.h"
#include "gfx/vulkan/surface.h"
#include <cstring>
#include <queue>
#include <set>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            bool physical_device::extension_is_supported(const char* extension_name) const noexcept
            {
                if (extension_name == nullptr)
                {
                    logger::warn("Extension name is nullptr");
                    return false;
                }
                
                for (const auto& extension_property : _info.extensions)
                {
                    if (strcmp(extension_name, extension_property.extensionName) == 0)
                    {
                        return true;
                    }
                }

                return false;
            }

            void physical_device::set_properties_() noexcept
            {
                vkGetPhysicalDeviceProperties(_info.physical_device, &_info.properties);
            }

            void physical_device::set_features_() noexcept
            {
                vkGetPhysicalDeviceFeatures(_info.physical_device, &_info.features);
            }

            void physical_device::set_memory_properties_() noexcept
            {
                vkGetPhysicalDeviceMemoryProperties(_info.physical_device, &_info.memory_properties);
            }

            bool physical_device::set_extensions_() noexcept
            {
                u32 extension_count = 0;
                const char* layer_name = nullptr;
                VkExtensionProperties* dummy_properties = nullptr;

                VkResult result = vkEnumerateDeviceExtensionProperties(
                    handle()
                    , layer_name
                    , &extension_count
                    , dummy_properties
                );

                if (result != VK_SUCCESS)
                {
                    logger::error("Failed to get extensions for physical device");
                    return false;
                }

                _info.extensions.resize(extension_count);
                result = vkEnumerateDeviceExtensionProperties(
                    handle()
                    , layer_name
                    , &extension_count
                    , _info.extensions.data()
                );

                if (result != VK_SUCCESS)
                {
                    logger::error("Failed to get extensions for physical device");
                    return false;
                }

                return true;
            }

            std::vector<VkDeviceQueueCreateInfo> physical_device::get_queue_family_infos() const noexcept
            {
                const f32 queue_priority = 1.0f;
                std::vector<VkDeviceQueueCreateInfo> create_infos {};
                std::set<u32> unique_queue_families {};

                if (_info.queue_family_indices.graphics.has_value()) 
                    unique_queue_families.insert(_info.queue_family_indices.graphics.value());
                if (_info.queue_family_indices.compute.has_value()) 
                    unique_queue_families.insert(_info.queue_family_indices.compute.value());
                if (_info.queue_family_indices.transfer.has_value()) 
                    unique_queue_families.insert(_info.queue_family_indices.transfer.value());

                for (const auto& queue_famliy_index : unique_queue_families)
                {
                    VkDeviceQueueCreateInfo queue_info {
                        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                        .flags = 0,
                        .queueFamilyIndex = queue_famliy_index,
                        .queueCount = 1,
                        .pQueuePriorities = &queue_priority
                    };
                    create_infos.push_back(queue_info);
                }

                return create_infos;
            }

            const std::optional<u32> physical_device::present_queue_index(const struct surface& surface) noexcept
            {
                if (_info.queue_family_indices.present.has_value())
                    return _info.queue_family_indices.present;
                
                if (_info.queue_families.empty())
                {
                    u32 queue_family_count = 0;
                    VkQueueFamilyProperties* dummy_properties = nullptr;

                    vkGetPhysicalDeviceQueueFamilyProperties(_info.physical_device, &queue_family_count, dummy_properties);
                    _info.queue_families.resize(queue_family_count);
                    vkGetPhysicalDeviceQueueFamilyProperties(_info.physical_device, &queue_family_count, _info.queue_families.data());
                }

                for (usize i = 0; i < _info.queue_families.size(); i++)
                {
                    VkBool32 present_support = VK_FALSE;
                    vkGetPhysicalDeviceSurfaceSupportKHR(handle(), i, surface.vk_surface, &present_support);
                    if (present_support)
                    {
                        _info.queue_family_indices.present = i;
                        return static_cast<u32>(i);
                    }
                }

                return std::nullopt;
            }

            void physical_device::find_queue_family_indices_() noexcept
            {
                u32 queue_family_count = 0;
                VkQueueFamilyProperties* dummy_properties = nullptr;

                vkGetPhysicalDeviceQueueFamilyProperties(_info.physical_device, &queue_family_count, dummy_properties);
                _info.queue_families.resize(queue_family_count);
                vkGetPhysicalDeviceQueueFamilyProperties(_info.physical_device, &queue_family_count, _info.queue_families.data());

                for (usize i = 0; i < _info.queue_families.size(); i++)
                {
                    const VkQueueFamilyProperties& properties = _info.queue_families[i];
                    if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                        _info.queue_family_indices.graphics = i;
                    if (properties.queueFlags & VK_QUEUE_TRANSFER_BIT)
                        _info.queue_family_indices.transfer = i;
                    if (properties.queueFlags & VK_QUEUE_COMPUTE_BIT)
                        _info.queue_family_indices.compute = i;
                }
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace
