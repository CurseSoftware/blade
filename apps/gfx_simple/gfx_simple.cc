#include "gfx/handle.h"
#include <blade/blade.h>

namespace logger = blade::logger;
namespace gfx = blade::gfx;
namespace fs = blade::resources::fs;

int main(void)
{
    const auto width = blade::width(800);
    const auto height = blade::height(800);
    
    auto window_opt = blade::window::create(
        "Blade Window",
        width,
        height
    );

    auto window = std::move(window_opt.value());

    gfx::init_info init {};
    init.type = gfx::init_info::type::VULKAN;
    init.resolution.width = 800;
    init.resolution.height = 600;
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
    
    gfx->shutdown();

    logger::info("GFX Example");
    
    return 0;
}
