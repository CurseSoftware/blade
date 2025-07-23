#include "gfx/vulkan/common.h"
// #include "gfx/vulkan/types.h"
#include "gfx/vulkan/instance.h"
#include "gfx/vulkan/utils.h"
#include "gfx/vulkan/platform.h"
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

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
 
            void instance::destroy() noexcept
            {
                if (_info.instance)
                {
                    logger::info("Destroying vulkan instance...");
                    vkDestroyInstance(_info.instance, nullptr);
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
//            void instance::destroy_debug_messenger() noexcept
//            {
//#ifdef BLADE_DEBUG
//                if (debug_messenger)
//                {
//                    logger::info("Destroying vulkan debug messenger...");
//                    PFN_vkDestroyDebugUtilsMessengerEXT func =
//                        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
//                    func(instance, debug_messenger, allocator);
//                    logger::info("Destroyed.");
//                }
//#endif
//            }

        } // vk namespace
    } // gfx namespace
} // blade namespace
