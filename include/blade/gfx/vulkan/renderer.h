#include "gfx/handle.h"
#include "gfx/renderer.h"
#include "gfx/view.h"
#include "gfx/program.h"
#include "gfx/vulkan/renderpass.h"
#include "gfx/vulkan/types.h"
#include <unordered_map>
#include <vulkan/vulkan_core.h>

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
                    ~vulkan_backend() noexcept;
                    

                    bool init(const init_info&) noexcept override;
                    bool shutdown() noexcept override;
                    void submit() noexcept override;
                    void frame() noexcept override;
                    framebuffer_handle create_framebuffer(framebuffer_create_info) noexcept override;

                    shader_handle create_shader(const std::vector<u8>&) noexcept override;

                    program_handle create_view_program(const framebuffer_handle, const shader_handle, const shader_handle) noexcept override;

                    struct view
                    {
                        std::weak_ptr<class device> device                        {};
                        std::shared_ptr<struct surface> surface                   { nullptr };
                        std::optional<std::unique_ptr<class swapchain>> swapchain { std::nullopt };
                        std::vector<VkFramebuffer> framebuffers                   {};
                        VkAllocationCallbacks* allocation_callbacks               { nullptr };
                        std::unique_ptr<class pipeline::builder> pipeline_builder { nullptr };
                        std::shared_ptr<class pipeline> graphics_pipeline         { nullptr };
                        struct program program                                    {};

                        void destroy() noexcept;
                        bool create_framebuffers(std::weak_ptr<class renderpass> renderpass) noexcept;
                    };
                private:
                    /// @brief Append platform-specific vulkan extensions to the list
                    std::vector<const char*> get_platform_extensions() const noexcept;

                    /// @brief Get the validation layer names
                    std::optional<std::vector<const char*>> get_debug_validation_layers() const noexcept;

                private:
                    bool _is_initialized                                       { false };
                    std::shared_ptr<instance> _instance                        { nullptr };
                    std::shared_ptr<class device> _device                      { nullptr };
                    std::shared_ptr<class renderpass> _main_renderpass         { nullptr };
                    VkAllocationCallbacks* allocation_callbacks                { nullptr };
                    std::unordered_map<framebuffer_handle, view> _views        {};
                    std::unordered_map<shader_handle, shader> _shaders         {};
                    std::unordered_map<program_handle, program> _programs      {};
            };
        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_VULKAN_H
