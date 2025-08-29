#include "gfx/renderer.h"
#include "gfx/handle.h"
#include "gfx/vertex.h"
#include "gfx/view.h"
#include "gfx/vulkan/renderer.h"

#include <memory>

namespace blade
{
    namespace gfx
    {
        std::unique_ptr<renderer> renderer::create(const init_info& init) noexcept
        {
            std::unique_ptr<renderer> renderer { new class renderer() };
            
            switch (init.type)
            {
                case init_info::type::VULKAN:
                {
                    auto backend = std::make_unique<vk::vulkan_backend>();
                    if (!backend->init(init))
                    {
                        logger::fatal("Failed to create vulkan backend");
                        return nullptr;
                    }
                    renderer->_backend = std::move(backend);

                    logger::info("Renderer initialized with vulkan backend");

                    return std::move(renderer);
                } break;
                
                case init_info::type::DX12:
                case init_info::type::METAL:
                case init_info::type::AUTO:
                    break;
            }
            
            return nullptr;
        }

        void renderer::submit() noexcept
        {
            if (_backend)
            {
                _backend->submit();
            }
        }

        void renderer::present() noexcept
        {
            if (_backend)
            {
                _backend->frame();
            }
        }

        framebuffer_handle renderer::create_framebuffer(framebuffer_create_info info) noexcept
        {
            if (_backend)
            {
                return _backend->create_framebuffer(info);
            }

            return framebuffer_handle { BLADE_NULL_HANDLE };
        }

        shader_handle renderer::create_shader(const std::vector<u8>& data) noexcept
        {
            if (_backend)
            {
                return _backend->create_shader(data);
            }

            return shader_handle { BLADE_NULL_HANDLE };
        }

        program_handle renderer::create_view_program(const framebuffer_handle framebuffer, const shader_handle vertex, const shader_handle fragment) noexcept
        {
            if (_backend)
            {
                return _backend->create_view_program(framebuffer, vertex, fragment);
            }

            return program_handle { BLADE_NULL_HANDLE };
        }

        buffer_handle renderer::create_vertex_buffer(const core::memory* memory, const vertex_layout& layout) noexcept
        {
            if (_backend)
            {
                return _backend->create_vertex_buffer(memory, layout);
            }

            return buffer_handle { BLADE_NULL_HANDLE };
        }

        void renderer::attach_vertex_buffer(const buffer_handle handle) const noexcept
        {
            if (_backend)
            {
                _backend->attach_vertex_buffer(handle);
            }
        }

        void renderer::set_viewport(const framebuffer_handle framebuffer, f32 x, f32 y, struct width width, struct height height) const noexcept
        {
            if (_backend)
            {
                _backend->set_viewport(framebuffer, x, y, width, height);
            }
        }

        void renderer::shutdown() noexcept
        {
            _backend->shutdown();
        }
    } // gfx namespace
} // blade namespace
