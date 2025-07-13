#include "gfx/vulkan/renderer.h"

namespace blade
{
    namespace gfx
    {
        namespace vk
        {
            vulkan_backend::vulkan_backend() noexcept
            {
                logger::debug("vulkan_backend constructor called");
            }

            vulkan_backend::~vulkan_backend()
            {
                logger::debug("vulkan_backend destructor called");
                if (_is_initialized)
                {
                    logger::warn("Vulkan backend destroying without explicit shutdown. Shutting down now.");
                    shutdown();
                }
            }

            bool vulkan_backend::init(const init_info& init) noexcept
            {
                logger::info("Initializing vulkan backend.");

                auto instance_opt = instance::create();
                instance = instance_opt.value();

                instance.create_debug_messenger();

                _is_initialized = true;
                return true;
            }

            bool vulkan_backend::shutdown() noexcept
            {
                logger::info("Vulkan backend shutting down");

                instance.destroy_debug_messenger();
                instance.destroy();

                _is_initialized = false;
                return true;
            }

            void vulkan_backend::frame() noexcept
            {
            }
        } // vk namespace
    } // gfx namespace
} // blade namespace
