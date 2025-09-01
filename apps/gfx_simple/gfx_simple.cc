#include "core/logger.h"
#include "core/memory.h"
#include "core/types.h"
#include "gfx/handle.h"
#include "gfx/vertex.h"
#include "math/operations.h"
#include "math/vec2.h"
#include <array>
#include <blade/blade.h>
#include <cstddef>

namespace logger = blade::logger;
namespace gfx = blade::gfx;
namespace fs = blade::resources::fs;
namespace math = blade::math;

struct vertex
{
    float positions[3] = { 0.f, 0.f, 0.f };
    float color[3] = { 0.f, 0.f, 0.f };
};

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

    constexpr blade::usize SIZEOF_POSITION = 3;
    blade::gfx::vertex_layout v_layout = 
        blade::gfx::vertex_layout().begin().value()
        .get()
        .add("position", SIZEOF_POSITION, gfx::attribute::datatype::f32, gfx::vertex_semantic::position)
        .add("color", SIZEOF_POSITION, gfx::attribute::datatype::f32, gfx::vertex_semantic::color)
        .end();

    std::vector<vertex> vertices = {
        {  {  0.0f, -0.5f,  0.0f}, { 1.0f, 0.0f, 0.0f } }, // TOP FRONT RIGHT
        {  {  0.5f,  0.5f,  0.0f}, { 0.0f, 1.0f, 0.0f } }, // TOP FRONT LEFT 
        {  { -0.5f,  0.5f,  0.0f}, { 0.0f, 0.0f, 1.0f } }, // TOP BACK LEFT
    };

    blade::core::memory positions_mem = {
        .data = vertices.data(),
        .size = vertices.size() * sizeof(vertex)
    };

    auto buffer_handle = gfx->create_vertex_buffer(&positions_mem, v_layout);
    gfx->attach_vertex_buffer(buffer_handle);

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

        gfx->set_vertex_buffer(buffer_handle);

        gfx->present();
    }

    
    gfx->shutdown();

    logger::info("GFX Example");
    
    return 0;
}
