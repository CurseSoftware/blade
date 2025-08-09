/* blade/gfx/renderer.h
 * 
 * This contains the base interface for a gfx renderer
 */

#ifndef BLADE_GFX_RENDERER_H
#define BLADE_GFX_RENDERER_H

#include "core/core.h"
#include "gfx/handle.h"
#include "gfx/view.h"
#include <memory>

namespace blade
{
    namespace gfx
    {
        struct resolution
        {
            u32 width { 0 };
            u32 height { 0 };

            enum class reset
            {
                VSYNC
            } reset { resolution::reset::VSYNC };
        };
        
        struct init_info
        {
            /// @brief The kind of renderer that we want to use
            enum class type
            {
                VULKAN,
                DX12,
                METAL,

                AUTO
            } type { init_info::type::AUTO };
          
            // Enable debug behavior
            bool enable_debug { false };

            /** @brief Require a surface like a window to render to */
            bool headless { false };

            /** @brief Resolution information */
            struct resolution resolution {};
        };
        
        /// @brief Abstraction over all renderer backends
        class renderer_backend
        {
            public:
                [[nodiscard]] renderer_backend() noexcept = default;

                virtual bool init(const init_info& init) noexcept = 0;
                virtual bool shutdown() noexcept = 0;
                virtual void frame() noexcept = 0;
                virtual framebuffer_handle create_framebuffer(framebuffer_create_info create_info) noexcept = 0;
                virtual shader_handle create_shader(const std::vector<u8>& mem) noexcept = 0;
                virtual program_handle create_view_program(const framebuffer_handle framebuffer, const shader_handle vertex, const shader_handle fragment) noexcept = 0;
                virtual void set_viewport(const framebuffer_handle framebuffer, u32 x, u32 y, struct width width, struct height height) noexcept = 0;

                virtual void submit() = 0;

                // TODO: other renderer backend methods
            
            private:
        };

        /// @brief User facing GFX interface
        class renderer
        {
            public:
                static std::unique_ptr<renderer> create(const init_info& init) noexcept;

                void shutdown() noexcept;

                framebuffer_handle create_framebuffer(framebuffer_create_info create_info) noexcept;

                shader_handle create_shader(const std::vector<u8>& mem) noexcept;

                program_handle create_view_program(const framebuffer_handle framebuffer, const shader_handle vertex, const shader_handle fragment) noexcept;

                void set_viewport(const framebuffer_handle framebuffer, u32 x, u32 y, struct width width, struct height height) const noexcept;

                void submit() noexcept;
                

            private:
                [[nodiscard]] explicit renderer() {}
                
                std::unique_ptr<renderer_backend> _backend { nullptr };
        };
    }
}

#endif // BLADE_GFX_RENDERER_H
