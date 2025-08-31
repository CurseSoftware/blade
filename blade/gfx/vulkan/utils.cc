#include "gfx/vulkan/utils.h"
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            const char* vk_vertex_format_str(VkFormat format) noexcept
            {
                switch (format)
                {
                    case VK_FORMAT_R8_UINT:
                        return "VK_FORMAT_R8_UINT";

                    case VK_FORMAT_R8_UNORM:
                        return "VK_FORMAT_R8_UNORM";

                    case VK_FORMAT_R8G8_UINT:
                        return "VK_FORMAT_R8G8_UINT";

                    case VK_FORMAT_R8G8_UNORM:
                        return "VK_FORMAT_R8G8_UNORM";

                    case VK_FORMAT_R8G8B8A8_UINT:
                        return "VK_FORMAT_R8G8B8A8_UINT";

                    case VK_FORMAT_R8G8B8A8_UNORM:
                        return "VK_FORMAT_R8G8B8A8_UNORM";

                    case VK_FORMAT_R16_SINT:
                        return "VK_FORMAT_R16_SINT";

                    case VK_FORMAT_R16_SNORM:
                        return "VK_FORMAT_R16_SNORM";

                    case VK_FORMAT_R16G16_SINT:
                        return "VK_FORMAT_R16G16_SINT";

                    case VK_FORMAT_R16G16_SNORM:
                        return "VK_FORMAT_R16G16_SNORM";

                    case VK_FORMAT_R16G16B16_SINT:
                        return "VK_FORMAT_R16G16B16_SINT";

                    case VK_FORMAT_R16G16B16_SNORM:
                        return "VK_FORMAT_R16G16B16_SNORM";

                    case VK_FORMAT_R16G16B16A16_SINT:
                        return "VK_FORMAT_R16G16B16A16_SINT";

                    case VK_FORMAT_R16G16B16A16_SNORM:
                        return "VK_FORMAT_R16G16B16A16_SNORM";

                    case VK_FORMAT_R32_SFLOAT:
                        return "VK_FORMAT_R32_SFLOAT";

                    case VK_FORMAT_R32G32_SFLOAT:
                        return "VK_FORMAT_R32G32_SFLOAT";

                    case VK_FORMAT_R32G32B32_SFLOAT:
                        return "VK_FORMAT_R32G32B32_SFLOAT";

                    case VK_FORMAT_R32G32B32A32_SFLOAT:
                        return "VK_FORMAT_R32G32B32A32_SFLOAT";

                    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
                        return "VK_FORMAT_A2R10G10B10_UINT_PACK32";

                    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
                        return "VK_FORMAT_A2R10G10B10_UNORM_PACK32";

                    case VK_FORMAT_R16_SFLOAT:
                        return "VK_FORMAT_R16_SFLOAT";

                    case VK_FORMAT_R16G16_SFLOAT:
                        return "VK_FORMAT_R16G16_SFLOAT";
                    
                    case VK_FORMAT_R16G16B16_SFLOAT:
                        return "VK_FORMAT_R16G16B16_SFLOAT";

                    case VK_FORMAT_R16G16B16A16_SFLOAT:
                        return "VK_FORMAT_R16G16B16A16_SFLOAT";

                    default:
                        return "INVALID VK FORMAT";
                }

                return "INVALID VK FORMAT";
            }
        } // vk namespace
    } // gfx namespace

} // blade namespace
