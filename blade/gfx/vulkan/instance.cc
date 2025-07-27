#include "gfx/vulkan/common.h"
// #include "gfx/vulkan/types.h"
#include "gfx/vulkan/instance.h"
#include "gfx/vulkan/utils.h"
#include "gfx/vulkan/platform.h"
#include <cstring>
#include <memory>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            const char* ENGINE_NAME = "Blade Engine";
            const char* APPLICATION_NAME = "Blade Application";

            std::optional<instance> instance::builder::build() const noexcept
            {
                u32 extension_count = static_cast<u32>(info.extension_names.size());
                u32 layer_count = static_cast<u32>(info.validation_layer_names.size());
                instance instance {};

                if (info.allocation_callbacks)
                {
                    instance._info.allocation_callbacks = info.allocation_callbacks;
                }

                if (!validate_extensions_())
                {
                    return std::nullopt;
                }

                if (info.validation_layer_names.size() > 0)
                {
                    u32 available_layer_count = 0;
                    std::vector<VkLayerProperties> available_layers {};
                    VkLayerProperties* dummy_layers = nullptr;

                    VkResult result = vkEnumerateInstanceLayerProperties(&available_layer_count, dummy_layers);
                    if (result != VK_SUCCESS)
                    {
                        return std::nullopt;
                    }

                    available_layers.resize(available_layer_count);
                    result = vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers.data());
                    if (result != VK_SUCCESS)
                    {
                        return std::nullopt;
                    }

                    for (const auto& layer_name: info.validation_layer_names)
                    {
                        bool found = false;
                        for (const auto& available_layer : available_layers)
                        {
                            if (strcmp(layer_name, available_layer.layerName) == 0)
                            {
                                found = true;
                                break;
                            }
                        }

                        if (!found)
                        {
                            logger::error("Validation layer {} is missing", layer_name);
                            return std::nullopt;
                        }
                    }

                    logger::info("Found all validation layers");
                }
                
                VkApplicationInfo app_info {
                    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                    .pApplicationName = info.app_name,
                    .pEngineName = info.engine_name,
                    .engineVersion = info.engine_version,
                    .apiVersion = info.api_version
                };

                VkInstanceCreateInfo instance_info {
                    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                    .pApplicationInfo = &app_info,
                    .enabledLayerCount = layer_count,
                    .ppEnabledLayerNames = extension_count > 0
                        ? info.validation_layer_names.data()
                        : nullptr,
                    .enabledExtensionCount = extension_count,
                    .ppEnabledExtensionNames = extension_count > 0
                        ? info.extension_names.data()
                        : nullptr,
                };

                VkResult r_instance = vkCreateInstance(&instance_info, info.allocation_callbacks, &instance._info.instance);
                if (r_instance != VK_SUCCESS)
                {
                    return std::nullopt;
                }
                
                return instance;
            };

            instance::builder& instance::builder::set_application_name(const char* app_name) noexcept
            {
                if (app_name)
                    info.app_name = app_name;

                return *this;
            }

            instance::builder& instance::builder::set_allocation_callbacks(VkAllocationCallbacks* callbacks) noexcept
            {
                info.allocation_callbacks = callbacks;

                return *this;
            }
            
            instance::builder& instance::builder::set_engine_name(const char* engine_name) noexcept
            {
                if (engine_name)
                    info.engine_name = engine_name;

                return *this;
            }

            instance::builder& instance::builder::request_validation_layers(const std::vector<const char*>& layers) noexcept
            {
                info.validation_layer_names = layers;
                return *this;
            }

            instance::builder& instance::builder::request_extensions(const std::vector<const char*>& extensions) noexcept
            {
                info.extension_names = extensions;

                return *this;
            }

            instance::builder& instance::builder::enable_debug_messenger() noexcept
            {
                info.enable_validation = true;

                return *this;
            }

            instance::builder& instance::builder::require_api_version(struct version version) noexcept
            {
                info.api_version = VK_MAKE_API_VERSION(
                    0
                    , version.major.value
                    , version.minor.value
                    , version.patch.value
                );
                return *this;
            }

            instance::builder& instance::builder::set_engine_version(struct version version) noexcept
            {
                info.engine_version = VK_MAKE_VERSION(
                    version.major.value
                    , version.minor.value
                    , version.patch.value
                );
                return *this;
            }

            instance::builder& instance::builder::request_default_validation_layers() noexcept
            {

                return *this;
            }

            bool instance::builder::validate_extensions_() const noexcept
            {
                u32 extension_count = 0;
                VkExtensionProperties *dummy_properties = nullptr;
                const char* layer_name = nullptr;
                std::vector<VkExtensionProperties> available_extensions {};

                logger::info("Validating required instance extensions...");
                
                VkResult result = vkEnumerateInstanceExtensionProperties(layer_name, &extension_count, dummy_properties);
                if (result != VK_SUCCESS)
                {
                    return false;
                }

                available_extensions.resize(extension_count);
                result = vkEnumerateInstanceExtensionProperties(layer_name, &extension_count, available_extensions.data());

                for (const auto& extension_name : info.extension_names)
                {
                    bool found = false;
                    logger::info("{}...", extension_name);
                    for (const auto& available_extension : available_extensions)
                    {
                        if (strcmp(extension_name, available_extension.extensionName) == 0)
                        {
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        logger::error("Required instance extension {} not found.", extension_name);
                        return false;
                    }
                    logger::info("Found.");
                }

                logger::info("All required instnace extensions found.");
                return true;
            }
 
            void instance::destroy() noexcept
            {
                if (_info.instance)
                {
                    logger::info("Destroying vulkan instance...");
                    vkDestroyInstance(_info.instance, nullptr);
                    logger::info("Destroyed.");
                }
            }

            std::vector<VkPhysicalDevice> instance::enumerate_physical_devices() const noexcept
            {
                VkPhysicalDevice* dummy_physical_device_list = nullptr;
                std::vector<VkPhysicalDevice> physical_devices {};
                u32 device_count = 0;

                VkResult result = vkEnumeratePhysicalDevices(handle(), &device_count, dummy_physical_device_list);
                if (result != VK_SUCCESS)
                {
                    logger::error("Error enumerating physical devices.");
                    return physical_devices;
                }

                physical_devices.resize(device_count);

                result = vkEnumeratePhysicalDevices(handle(), &device_count, physical_devices.data());
                if (result != VK_SUCCESS)
                {
                    logger::error("Error enumerating physical devices.");
                    return physical_devices;
                }

                return physical_devices;
            }
            void instance::destroy_debug_messenger() noexcept
            {
                if (_info.debug_messenger)
                {
                    logger::info("Destroying vulkan debug messenger...");
                    PFN_vkDestroyDebugUtilsMessengerEXT func =
                        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_info.instance, "vkDestroyDebugUtilsMessengerEXT");
                    func(_info.instance, _info.debug_messenger, _info.allocation_callbacks);
                    logger::info("Destroyed.");
                }
            }

//            std::optional<struct instance> instance::create(VkAllocationCallbacks* allocator) noexcept
//            {
//                VkApplicationInfo app_info {
//                    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
//                    .pApplicationName = APPLICATION_NAME,
//                    .pEngineName = ENGINE_NAME,
//                    .engineVersion = VK_MAKE_VERSION(1, 0, 0),
//                    .apiVersion = VK_API_VERSION_1_3
//                };
//
//                VkInstanceCreateInfo create_info {
//                    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
//                    .pApplicationInfo = &app_info
//                };
//
//                std::vector<const char*> extensions = { VK_KHR_SURFACE_EXTENSION_NAME };
//                
//                (void) get_platform_extensions(extensions);
//
//#ifdef BLADE_DEBUG
//                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
//                logger::debug("Extensions:");
//                for (const auto& ext: extensions)
//                {
//                    logger::debug("\t{}", ext);
//                }
//#endif
//                create_info.enabledExtensionCount = static_cast<u32>(extensions.size());
//                create_info.ppEnabledExtensionNames = extensions.data();
//
//                std::vector<const char*> validation_layer_names = {};
//#ifdef BLADE_DEBUG
//                validation_layer_names.push_back("VK_LAYER_KHRONOS_validation");
//                logger::debug("Validation layers enabled. Enumerating: ");
//                u32 available_layer_count = 0;
//                std::vector<VkLayerProperties> available_layers {};
//
//                VK_ASSERT(vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr));
//                available_layers.resize(available_layer_count);
//
//                VK_ASSERT(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers.data()));
//
//                for (usize i = 0; i < validation_layer_names.size(); i++)
//                {
//                    const char* layer_name = validation_layer_names[i];
//                    bool found = false;
//                    
//                    logger::debug("Searching for validation layer {}", layer_name);
//
//                    for (usize j = 0; j < available_layer_count; j++)
//                    {
//                        const char* available_layer_name = available_layers[j].layerName;
//                        if (strcmp(layer_name, available_layer_name))
//                        {
//                            found = true;
//                            logger::debug("Found.");
//                            break;
//                        }
//                    }
//
//                    if (!found)
//                    {
//                        logger::error("Required validation layer is missing: {}", layer_name);
//                        return std::nullopt;
//                    }
//                }
//#endif
//
//                create_info.enabledLayerCount = static_cast<u32>(validation_layer_names.size());
//                create_info.ppEnabledLayerNames = validation_layer_names.data();
//
//                struct instance inst {};
//
//                inst.validation_layers = validation_layer_names;
//                inst.allocator = allocator;
//
//                VK_ASSERT(vkCreateInstance(&create_info, inst.allocator, &inst.instance));
//
//                logger::info("Vulkan instance created");
//
//                return inst;
//            }
//

        } // vk namespace
    } // gfx namespace
} // blade namespace
