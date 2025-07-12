#include <blade/blade.h>

namespace events = blade::events;
namespace logger = blade::logger;

int main(void) 
{
    auto window_opt = blade::window::create(
        "Blade Window",
        blade::width(800),
        blade::height(600)
    );

    auto window = std::move(window_opt.value());

    events::subscribe<events::window_resize<blade::window>>([](const events::window_resize<blade::window>& e) {
        logger::trace("Window '{}' resized ({}, {})", e.window.get_title(), e.width, e.height);
        return true;
    });

    window->show();
    
    while (!window->should_close())
    {

    }

    return 0;
}
