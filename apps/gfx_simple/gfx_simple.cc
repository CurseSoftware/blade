#include <blade/blade.h>

namespace logger = blade::logger;

int main(void)
{
    auto window_opt = blade::window::create(
        "Blade Window",
        blade::width(800),
        blade::height(600)
    );

    auto window = std::move(window_opt.value());

    logger::info("GFX Example");
    
    return 0;
}
