#include "gfx/renderer.h"
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

        void renderer::shutdown() noexcept
        {
            _backend->shutdown();
        }
    } // gfx namespace
} // blade namespace
