#ifndef BLADE_GFX_VULKAN_INTANCE
#define BLADE_GFX_VULKAN_INTANCE

#include "gfx/vulkan/common.h"
#include <optional>
#include <memory>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            inline constexpr const char* validation_layer_name = "VK_LAYER_KHRONOS_validation";

            struct version
            {
                struct { u32 value = 0; } major;
                struct { u32 value = 0; } minor ;
                struct { u32 value = 0; } patch;
            };

            class instance
            {
            public:
                class builder
                {
                public:
                    builder() {}

                    builder& set_application_name(const char* app_name) noexcept;
                    builder& set_engine_name(const char* engine_name) noexcept;
                    builder& enable_debug_messenger() noexcept;
                    builder& require_api_version(struct version v = version{.major{1}, .minor {0}, .patch {0}}) noexcept;
                    builder& set_engine_version(struct version v = version{.major{1}, .minor {0}, .patch {0}}) noexcept;
                    builder& request_default_validation_layers() noexcept;
                    builder& request_validation_layers(const std::vector<const char*>& layers) noexcept;
                    builder& request_extensions(const std::vector<const char*>& extensions) noexcept;
                    builder& set_allocation_callbacks(VkAllocationCallbacks* callbacks) noexcept;

                    std::optional<instance> build() const noexcept;

                private:
                    bool validate_extensions_() const noexcept;

                private:

                    struct
                    {
                        VkAllocationCallbacks *allocation_callbacks     { nullptr };
                        const char* app_name                            { "" };
                        const char* engine_name                         { "" };
                        bool enable_validation                          { false };
                        u32 engine_version                              { 0 };
                        u32 api_version                                 { 0 };
                        std::vector<const char*> validation_layer_names {};
                        std::vector<const char*> extension_names        {};
                    } info {};
                };
                
                [[nodiscard]] explicit instance(VkInstance instance) : _info{instance} {}
                [[nodiscard]] explicit instance() {}
                
                instance(const instance& other) = delete;
                instance& operator=(const instance& other) = delete;


                [[nodiscard]] instance(instance&& other) noexcept 
                {
                    if (this != &other)
                    {
                        this->_info.instance = std::exchange(other._info.instance, VK_NULL_HANDLE);
                    }
                }
                
                instance& operator=(instance&& other) noexcept
                {
                    if (this != &other)
                    {
                        this->_info.instance = std::exchange(other._info.instance, VK_NULL_HANDLE);
                    }

                    return *this;
                }

                /** 
                 * @brief Enumerate available physical devices 
                 * @return Vector of `VkPhysicalDevice` of available devices
                 */
                std::vector<VkPhysicalDevice> enumerate_physical_devices() const noexcept;

                VkInstance handle() const { return _info.instance; }
                VkAllocationCallbacks* allocation_callbacks() const { return _info.allocation_callbacks; }

                void create_debug_messenger() noexcept;
                void destroy_debug_messenger() noexcept;
                void destroy() noexcept;

            private:
                struct
                {
                    VkInstance instance                         { VK_NULL_HANDLE };
                    VkAllocationCallbacks *allocation_callbacks { nullptr };
                    VkDebugUtilsMessengerEXT debug_messenger;
                } _info {};
            };
        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_INTANCE
