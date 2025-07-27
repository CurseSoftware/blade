#include "gfx/vulkan/types.h"
#include "gfx/vulkan/utils.h"
#include <memory>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            /// @brief Debug callback for vulkan
            /// @param message_severity Severity of the debug message
            /// @param message_type Type of message
            /// @param p_callback_data Data packaged with the message
            static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
                VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                VkDebugUtilsMessageTypeFlagsEXT message_type,
                const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
                void* p_user_data
            ) {
                switch(message_severity)
                {
                    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                    {
                        logger::warn("Validation layer: {}", p_callback_data->pMessage);
                    } break;
                    
                    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                    {
                        logger::error("Validation layer: {}", p_callback_data->pMessage);
                    } break;
                    
                    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                    {
                        logger::info("Validation layer: {}", p_callback_data->pMessage);
                    } break;

                    default:
                        break;
                }
                
                return VK_FALSE;
            }

            void instance::create_debug_messenger() noexcept
            {
                u32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                 ;
                
                u32 message_type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                 ;

                VkDebugUtilsMessengerCreateInfoEXT create_info {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                    .messageSeverity = log_severity,
                    .messageType = message_type,
                    .pfnUserCallback = debug_callback,
                    .pUserData = nullptr,
                };

                PFN_vkCreateDebugUtilsMessengerEXT func =
                    (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_info.instance, "vkCreateDebugUtilsMessengerEXT");

                VK_ASSERT(func(_info.instance, &create_info, _info.allocation_callbacks, &_info.debug_messenger));
                logger::debug("Vulkan debug messenger created");
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace
