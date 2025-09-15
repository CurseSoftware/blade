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

                VkDeviceCreateInfo create_info{
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

            device::builder& device::builder::set_allocation_callbacks(VkAllocationCallbacks* callbacks) noexcept
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
                std::vector<std::shared_ptr<physical_device>> physical_devices{};
                for (const auto& device : vk_physical_devices)
                {
                    auto pd = std::make_shared<physical_device>(device);
                    physical_devices.push_back(pd);
                }

                std::erase_if(physical_devices, [this](std::shared_ptr<physical_device> device) -> bool
                {
                    constexpr bool should_erase = true;
                    constexpr bool should_not_erase = false;

                    logger::info("Checking device {}", device->name());

                    for (auto extension : info.required_extensions)
                    {
                        logger::info("Checking extension {}", extension);
                        if (!device->extension_is_supported(extension))
                        {
                            logger::info("Physical Device {} does not support required extension {}. Removing.",
                                         device->name(), extension);
                            return should_erase;
                        }
                        logger::info("Physical Device {} supports required extension {}", device->name(), extension);

                        if (info.require_graphics_queue && !device->graphics_queue_index().has_value())
                        {
                            logger::info("Physical Device {} does not have required graphics queue. Removing.",
                                         device->name());
                            return should_erase;
                        }

                        if (info.require_transfer_queue && !device->transfer_queue_index().has_value())
                        {
                            logger::info("Physical Device {} does not have required transfer queue. Removing.",
                                         device->name());
                            return should_erase;
                        }

                        if (info.require_compute_queue && !device->compute_queue_index().has_value())
                        {
                            logger::info("Physical Device {} does not have required compute queue. Removing.",
                                         device->name());
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
                VkQueue queue{};
                auto index_opt = get_queue_index(type);
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

            std::optional<u32> device::get_queue_index(const queue_type type) const noexcept
            {
                switch (type)
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
                VkSurfaceCapabilitiesKHR capabilities{};
                auto handle = _physical_device->handle();

                // TODO: check error value here
                // potentially return std::optional from this
                (void)vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
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
                std::vector<VkSurfaceFormatKHR> formats{};
                (void)vkGetPhysicalDeviceSurfaceFormatsKHR(
                    _physical_device->handle()
                    , surface.vk_surface
                    , &format_count
                    , dummy_formats
                );

                formats.resize(format_count);

                (void)vkGetPhysicalDeviceSurfaceFormatsKHR(
                    _physical_device->handle()
                    , surface.vk_surface
                    , &format_count
                    , formats.data()
                );

                return formats;
            }

            const std::vector<VkPresentModeKHR> device::surface_present_modes(
                const struct surface& surface) const noexcept
            {
                u32 present_mode_count = 0;
                VkPresentModeKHR* dummy_present_modes = nullptr;
                std::vector<VkPresentModeKHR> present_modes{};

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
        } // vk namespace
    } // gfx namespace
} // blade namespace
