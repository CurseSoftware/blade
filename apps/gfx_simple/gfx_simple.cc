#include "core/types.h"
#include "gfx/handle.h"
#include "gfx/vertex.h"
#include "math/operations.h"
#include "math/vec2.h"
#include <array>
#include <blade/blade.h>

namespace logger = blade::logger;
namespace gfx = blade::gfx;
namespace fs = blade::resources::fs;
namespace math = blade::math;

int main(void)
{
    constexpr blade::u32 FRAME_WIDTH = 800;
    constexpr blade::u32 FRAME_HEIGHT = 600;
    
    const auto width = blade::width(FRAME_WIDTH);
    const auto height = blade::height(FRAME_HEIGHT);
    
    auto window_opt = blade::window::create(
        "Blade Window",
        width,
        height
    );

    math::vec3<float> a = { 1.0, 2.0, 3.0 };
    math::vec3<float> b = { 5.0, 6.0, 7.0 };
    logger::trace("CROSS(a,b): {}", math::cross(a, b).to_string());

    auto window = std::move(window_opt.value());

    gfx::init_info init {};
    init.type = gfx::init_info::type::VULKAN;
    init.resolution.width = FRAME_WIDTH;
    init.resolution.height = FRAME_HEIGHT;
    init.resolution.reset = gfx::resolution::reset::VSYNC;
    init.enable_debug = true;
    init.headless = false;

    auto gfx = gfx::renderer::create(init);
    if (!gfx)
    {
        logger::fatal("Failed to create renderer");
        return 1;
    }

    auto frame = gfx->create_framebuffer({
        .native_window_data = window->get_window_handle(),
        .width = width,
        .height = height
    });

    auto vert_opt = fs::file::from_path("simple.vert.spv", fs::file_mode::read);
    auto frag_opt = fs::file::from_path("simple.frag.spv", fs::file_mode::read);

    if (!vert_opt.has_value())
    {
        logger::error("Vertex file not read properly");
        return 1;
    }
    
    if (!frag_opt.has_value())
    {
        logger::error("Vertex file not read properly");
        return 1;
    }

    auto vert_file = std::move(vert_opt.value());
    auto frag_file = std::move(frag_opt.value());

    vert_file.open();
    frag_file.open();

    auto vert_code = vert_file.read_all().value();
    auto frag_code = frag_file.read_all().value();

    auto vert_handle = gfx->create_shader(vert_code);
    auto frag_handle = gfx->create_shader(frag_code);

    auto program = gfx->create_view_program(frame, vert_handle, frag_handle);

    if (program.index != blade::gfx::BLADE_NULL_HANDLE)
    {
        logger::debug("Valid program handle created");
    }

    window->show();

    while (!window->should_close())
    {
        gfx->set_viewport(frame, 0, 0, blade::width{ window->get_width() }, blade::height{ window->get_height() });

        gfx->present();
    }

    
    gfx->shutdown();

    logger::info("GFX Example");
    
    return 0;
}
