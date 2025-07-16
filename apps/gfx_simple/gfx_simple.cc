#include <blade/blade.h>

namespace logger = blade::logger;
namespace gfx = blade::gfx;

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

    auto gfx = gfx::renderer::create(init);
    if (!gfx)
    {
        logger::fatal("Failed to create renderer");
        return 1;
    }

    gfx->create_framebuffer({
        .native_window_data = window->get_window_handle(),
        .width = width,
        .height = height
    });

    gfx->shutdown();

    logger::info("GFX Example");
    
    return 0;
}
