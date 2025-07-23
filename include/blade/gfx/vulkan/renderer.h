#include "gfx/handle.h"
#include "gfx/renderer.h"
#include "gfx/view.h"
#include "gfx/vulkan/types.h"
#include <unordered_map>

#ifndef BLADE_GFX_VULKAN_VULKAN_H
#define BLADE_GFX_VULKAN_VULKAN_H

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            class vulkan_backend : public renderer_backend
            {
                public:
                    [[nodiscard]] vulkan_backend() noexcept;
                    ~vulkan_backend();

                    bool init(const init_info& init) noexcept override;
                    bool shutdown() noexcept override;
                    void frame() noexcept override;
                    framebuffer_handle create_framebuffer(framebuffer_create_info create_info) noexcept override;

                    struct view 
                    {
                        struct surface surface;
                        std::optional<struct swapchain> swapchain;

                        void destroy();
                    };
                private:
                    /// @brief Append platform-specific vulkan extensions to the list
                    std::vector<const char*> get_platform_extensions() const noexcept;

                    /// @brief Get the validation layer names
                    std::optional<std::vector<const char*>> get_debug_validation_layers() const noexcept;

                private:
                    bool _is_initialized { false };
                    instance _instance;
                    struct device _device {};
                    std::unordered_map<framebuffer_handle, view> _views {};
            };
        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_VULKAN_H
