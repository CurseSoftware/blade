#include "gfx/renderer.h"
#include "gfx/handle.h"
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

        void renderer::shutdown() noexcept
        {
            _backend->shutdown();
        }
    } // gfx namespace
} // blade namespace
