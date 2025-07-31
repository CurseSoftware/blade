#ifndef BLADE_GFX_VULKAN_DEVICE_H
#define BLADE_GFX_VULKAN_DEVICE_H

#include "gfx/vulkan/common.h"
#include <functional>
#include <memory>
#include <optional>
#include <utility>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            enum class queue_type
            {
                graphics,
                present,
                transfer,
                compute
            };

            class physical_device
            {
            public:
                physical_device(VkPhysicalDevice physical_device) 
                {
                    _info.physical_device = physical_device;
                    set_properties_();
                    set_features_();
                    set_extensions_();
                    set_memory_properties_();
                    find_queue_family_indices_();
                    
                    _info.name = _info.properties.deviceName;
                }

                physical_device(physical_device&& other) noexcept
                {
                    if (this != &other)
                    {
                        _info.name = other._info.name;
                        _info.device_id = other._info.device_id;
                        _info.physical_device = other._info.physical_device;
                        _info.properties = other._info.properties;
                        _info.features = other._info.features;
                        _info.memory_properties = other._info.memory_properties;
                        _info.extensions = other._info.extensions;
                        _info.queue_families = other._info.queue_families;

                        _info.queue_family_indices.graphics = other._info.queue_family_indices.graphics;
                        _info.queue_family_indices.transfer = other._info.queue_family_indices.transfer;
                        _info.queue_family_indices.present = other._info.queue_family_indices.present;
                        _info.queue_family_indices.compute = other._info.queue_family_indices.compute;
                    }
                }

                physical_device& operator=(physical_device&& other) noexcept
                {
                    if (this != &other)
                    {
                        _info.name = other._info.name;
                        _info.device_id = other._info.device_id;
                        _info.physical_device = other._info.physical_device;
                        _info.properties = other._info.properties;
                        _info.features = other._info.features;
                        _info.memory_properties = other._info.memory_properties;
                        _info.extensions = other._info.extensions;
                        _info.queue_families = other._info.queue_families;

                        _info.queue_family_indices.graphics = other._info.queue_family_indices.graphics;
                        _info.queue_family_indices.transfer = other._info.queue_family_indices.transfer;
                        _info.queue_family_indices.present = other._info.queue_family_indices.present;
                        _info.queue_family_indices.compute = other._info.queue_family_indices.compute;
                    }

                    return *this;
                }


                /**
                 * @brief Get the properties of this physical device
                 */
                const VkPhysicalDeviceProperties get_properties() const { return _info.properties; }
                
                /**
                 * @brief Get the features of this physical device
                 */
                const VkPhysicalDeviceFeatures* get_features_ptr() const { return &_info.features; }
               
                /**
                 * @brief Check if an extension is supported by the physical device
                 * @return `true` if supported and `false` otherwise
                 */
                bool extension_is_supported(const char* extension) const noexcept;

                /**
                 * @brief get the handle to the `VkPhysicalDevice`
                 * @return handle to the `VkPhysicalDevice`
                 */
                const VkPhysicalDevice& handle() const noexcept { return _info.physical_device; }
                
                /**
                 * @brief Create and retrieve list of create info structs for queue famlilies
                 * @return std::vector of unique queue family create infos
                 */
                std::vector<VkDeviceQueueCreateInfo> get_queue_family_infos() const noexcept;

                /**
                 * @brief name getter for the physical device
                 * @return `const char*` of the name
                 */
                const char* name() const noexcept { return _info.name.c_str(); }

                const std::optional<u32> graphics_queue_index() const noexcept { return _info.queue_family_indices.graphics; }
                const std::optional<u32> transfer_queue_index() const noexcept { return _info.queue_family_indices.transfer; }
                const std::optional<u32> compute_queue_index() const noexcept { return _info.queue_family_indices.compute; }
                const std::optional<u32> present_queue_index(const struct surface&) noexcept;

            private:
                /**
                 * @brief Fill in the indices for each queue family on this device 
                 */
                void find_queue_family_indices_() noexcept;
                
            private:
                struct
                {
                    std::string name                                    {};
                    u32 device_id                                       {};
                    VkPhysicalDevice physical_device                    { VK_NULL_HANDLE };
                    VkPhysicalDeviceProperties properties               {};
                    VkPhysicalDeviceFeatures features                   {};
                    VkPhysicalDeviceMemoryProperties memory_properties  {};
                    std::vector<VkExtensionProperties> extensions       {};
                    std::vector<VkQueueFamilyProperties> queue_families {};

                    struct
                    {
                        std::optional<u32> graphics {};
                        std::optional<u32> transfer {};
                        std::optional<u32> compute  {};
                        std::optional<u32> present  {};
                    } queue_family_indices {};
                } _info;
                
                void set_properties_() noexcept;
                void set_features_() noexcept;
                void set_memory_properties_() noexcept;
                bool set_extensions_() noexcept;

                friend class ::std::hash<class physical_device>;
            };


            class device
            {
                public:
                    struct builder
                    {
                        [[nodiscard]] explicit builder(std::weak_ptr<class instance> instance) noexcept 
                            : info { instance }
                        {}

                        std::optional<std::shared_ptr<device>> build() const noexcept;

                        builder& set_allocation_callbacks(VkAllocationCallbacks* callbacks) noexcept;
                        builder& require_features(VkPhysicalDeviceFeatures physical_device_features) noexcept;
                        builder& require_extension(const char* extension) noexcept;
                        builder& require_queue(queue_type type) noexcept;

                        struct
                        {
                            std::weak_ptr<const class instance> instance {};
                            std::vector<const char*> required_extensions {};
                            VkPhysicalDeviceFeatures physical_device_features {};
                            VkAllocationCallbacks* allocation_callbacks       { nullptr };


                            bool require_graphics_queue { false };
                            bool require_transfer_queue { false };
                            bool require_present_queue  { false };
                            bool require_compute_queue  { false };
                        } info;

                        private:
                            std::vector<std::shared_ptr<physical_device>> find_valid_devices_() const noexcept;
                    };
                    
                    device(class device&& other) noexcept
                    {
                        if (this != &other)
                        {
                            _physical_device = std::move(other._physical_device);
                            _logical_device = std::exchange(other._logical_device, VK_NULL_HANDLE);
                        }
                    }
                    
                    device& operator=(device&& other) noexcept
                    {
                        if (this != &other)
                        {
                            _physical_device = std::move(other._physical_device);
                            _logical_device = std::exchange(other._logical_device, VK_NULL_HANDLE);
                        }

                        return *this;
                    }

                    std::optional<u32> graphics_queue_index() const noexcept { return _physical_device->graphics_queue_index(); }
                    std::optional<u32> present_queue_index(const struct surface& surface) noexcept { return _physical_device->present_queue_index(surface); }

                    std::optional<VkQueue> get_present_queue(VkSurfaceKHR surface) const noexcept;
                    std::optional<VkQueue> get_queue(const queue_type type) const noexcept;

                    VkSurfaceCapabilitiesKHR surface_capabilities(const struct surface&) const noexcept;
                    const std::vector<VkSurfaceFormatKHR> surface_formats(const struct surface&) const noexcept;
                    const std::vector<VkPresentModeKHR> surface_present_modes(const struct surface&) const noexcept;

                    const VkDevice handle() const noexcept { return _logical_device; }
                    const std::weak_ptr<const physical_device> get_physical_device() const noexcept { return _physical_device; }

                    void destroy() noexcept;

                
                    [[nodiscard]] explicit device(std::shared_ptr<physical_device> physical_device) :  _physical_device { physical_device } {}
                private:
                    
                    device(const device&) = delete;
                    device& operator=(const device&) = delete;

                    std::optional<u32> get_queue_index_(const queue_type type) const noexcept;

                private:
                    std::shared_ptr<physical_device> _physical_device       { nullptr };
                    VkDevice _logical_device                                { VK_NULL_HANDLE };
                    VkAllocationCallbacks* _allocation_callbacks            { nullptr };
            };

        } // vk namespace
    } // gfx namespace
} // blade namespace

template <>
struct std::hash<blade::gfx::vk::physical_device>
{
    std::size_t operator()(const blade::gfx::vk::physical_device pd) const
    {
        return std::hash<blade::u32>{}(pd._info.device_id);
    }
};

#endif // BLADE_GFX_VULKAN_DEVICE_H
