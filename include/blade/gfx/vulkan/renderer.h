#include "core/core.h"
#include "core/memory.h"
#include "gfx/handle.h"
#include "gfx/renderer.h"
#include "gfx/vertex.h"
#include "gfx/view.h"
#include "gfx/program.h"
#include "gfx/vulkan/buffer.h"
#include "gfx/vulkan/command.h"
#include "gfx/vulkan/view.h"
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
                    void set_viewport(const framebuffer_handle framebuffer, f32 x, f32 y, struct width width, struct height height) noexcept override;
                    void attach_vertex_buffer(const buffer_handle) noexcept override;
                    void set_vertex_buffer(const buffer_handle handle) noexcept override;
                    void set_index_buffer(const buffer_handle handle) noexcept override;

                    framebuffer_handle create_framebuffer(framebuffer_create_info) noexcept override;
                    shader_handle create_shader(const std::vector<u8>&) noexcept override;
                    program_handle create_view_program(const framebuffer_handle, const shader_handle, const shader_handle) noexcept override;
                    buffer_handle create_vertex_buffer(const core::memory* memory, const vertex_layout& layout) noexcept override;
                    buffer_handle create_index_buffer(const core::memory* memory) noexcept override;

                private:
                    /// @brief Append platform-specific vulkan extensions to the list
                    std::vector<const char*> get_platform_extensions() const noexcept;

                    /// @brief Get the validation layer names
                    std::optional<std::vector<const char*>> get_debug_validation_layers() const noexcept;

                private:
                    bool _is_initialized                                       { false };
                    std::shared_ptr<instance> _instance                        { nullptr };
                    std::shared_ptr<class device> _device                      { nullptr };
                    // std::shared_ptr<class command_pool> _command_pool          { nullptr };
                    VkAllocationCallbacks* allocation_callbacks                { nullptr };
                    std::unordered_map<framebuffer_handle, view> _views        {};
                    std::unordered_map<shader_handle, shader> _shaders         {};
                    std::unordered_map<program_handle, program> _programs      {};

                    std::shared_ptr<command_handler> _cmd_handler              { nullptr };
                    u16 _buffer_handle_index { 0 };

                    std::unordered_map<buffer_handle, std::shared_ptr<buffer>> _vertex_input_infos  {};
                    std::unordered_map<buffer_handle, std::shared_ptr<buffer>> _index_input_infos   {};
                    u32 _num_bindings { 0 };
            };
        } // vk namespace
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VULKAN_VULKAN_H
