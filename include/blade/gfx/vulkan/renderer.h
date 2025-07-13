#include "gfx/renderer.h"
#include "gfx/vulkan/types.h"

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

                private:
                    bool _is_initialized { false };
                    struct instance instance {};
                    struct device device {};
            };
        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_VULKAN_H
