#include "window/window.h"
#include "core/core.h"

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <memory>
#include <optional>

namespace blade
{
    const unsigned int WINDOW_MIN_WIDTH = 640;
    const unsigned int WINDOW_MIN_HEIGHT = 480;
    
    XContext window::_global_x11_context = 0;

    std::optional<std::unique_ptr<window>> window::create(
        const std::string& name,
        width w,
        height h
    ) {
        
        auto wnd = std::make_unique<window>(window(name, w, h));

        wnd->_display = XOpenDisplay(nullptr);

        logger::info("Creating window {} at dimensions ({}, {})", name, w.w, h.h);

        // Initialize the global window context with X11 only if it has not been before
        {
            if (_global_x11_context == 0)
            {
                _global_x11_context = XUniqueContext();
                if (!_global_x11_context)
                {
                    logger::error("Failed to create global X11 window context.");
                    return std::nullopt;
                }

                logger::info("Global X11 window context initialized");
            }
        }

        // Create the xlin window
        {
            const int x = 0;
            const int y = 0;
            const unsigned int width = static_cast<unsigned int>(w.w);
            const unsigned int height = static_cast<unsigned int>(h.h);
            const unsigned long border_width = 0;
            const unsigned long white = WhitePixel(wnd->_display, DefaultScreen(wnd->_display));
            const unsigned long background = white;
            const unsigned long border = white;
            
            wnd->_window = XCreateSimpleWindow(
                wnd->_display,
                DefaultRootWindow(wnd->_display),
                x,
                y,
                width,
                height,
                border_width,
                border,
                background
            );

            if (!wnd->_window) 
            {
                logger::fatal("Failed to create window. XCreateSimpleWindow failed");
                return std::nullopt;
            }
        }

        // Choose which events we want to listen for
        {
            const long event_mask = KeyPressMask
                                  | KeyReleaseMask
                                  | StructureNotifyMask
                                  | ExposureMask
                                  ;

            XSelectInput(
                wnd->_display,
                wnd->_window,
                event_mask
            );
        }

        // Request to be notified when window is deleted
        Atom wm_protocols;
        {
            const char* atom_name = "WM_DELETE_WINDOW";
            const bool only_if_exists = true;
            const int count = 1;
            
            wm_protocols = XInternAtom(wnd->_display, atom_name, only_if_exists);
            XSetWMProtocols(wnd->_display, wnd->_window, &wm_protocols, count);
        }

        // Set window properties
        {
            const char* icon_name = "Icon name";
            const Pixmap icon_pixmap = None;
            char** argv = nullptr;
            const int argc = 0;
            XSizeHints* size_hints = nullptr;
            XSetStandardProperties(
                wnd->_display,
                wnd->_window,
                name.c_str(),
                icon_name,
                icon_pixmap,
                argv,
                argc,
                size_hints
            );
        }

        // Set window size hints
        {
            XSizeHints size_hints;
            size_hints.flags = PMinSize;
            size_hints.min_width = WINDOW_MIN_WIDTH;
            size_hints.min_height = WINDOW_MIN_HEIGHT;

            const Atom property = XA_WM_NORMAL_HINTS;

            XSetWMSizeHints(wnd->_display, wnd->_window, &size_hints, property);
        }

        {
            const int result = XSaveContext(
                wnd->_display, 
                wnd->_window, 
                _global_x11_context, 
                reinterpret_cast<XPointer>(wnd.get())
            );

            if (result != 0)
            {
                logger::error("Failed to store window in global x11 context");
                return std::nullopt;
            }
        }

        wnd->_is_initialized = true;
        logger::info("Window '{}' created successfully", name);

        return std::move(wnd);
    }

    void window::show()
    {
        _should_close = false;
        XMapWindow(_display, _window);
        XFlush(_display);
    }

    void window::close()
    {
        if (_should_close)
        {
            return;
        }
        XDestroyWindow(_display, _window);
        XFlush(_display);
        _should_close = true;
    }

    bool window::should_close()
    {
        _pump_messages();
        return _should_close;
    }

    void window::shutdown()
    {
        events::dispatch(events::window_close {});
        
        if (!_is_initialized)
        {
            logger::info("Window '{}' already shutdown. Skipping", _title);
            return;
        }

        close();

        _is_initialized = false;
        logger::info("Window '{}' shutdown successfuly", _title);
    }

    void window::_handle_x11_event(XEvent& event)
    {
        window* window = nullptr;
        const int result = XFindContext(event.xany.display, event.xany.window, _global_x11_context, reinterpret_cast<XPointer*>(&window));
        if (result != 0 || window == nullptr)
        {
            logger::fatal("Failed to retrieve window instance from X11 event. Returning early");
            return;
        }
        
        switch(event.type)
        {
            case ClientMessage:
            {
                window->shutdown();
            } break;

            case ConfigureNotify:
            {
                const u32 new_width = static_cast<u32>(event.xconfigure.width);
                const u32 new_height = static_cast<u32>(event.xconfigure.height);
                if (new_width != window->_width || new_height != window->_height)
                {
                    window->_handle_resize(width(new_width), height(new_height));
                }
            } break;

            case KeyPress:
            case KeyRelease:
            {
            } break;

            case FocusIn:
            case FocusOut:
            {
            } break;

            default:
                break;
        }
    }

    void window::_handle_resize(struct width width, struct height height)
    {
        _width = width.w;
        _height = height.h;
        events::dispatch(events::window_resize(width, height, *this));
    }

    void window::_pump_messages()
    {
        XEvent event {};
        while (XPending(_display) > 0)
        {
            XNextEvent(_display, &event);
            _handle_x11_event(event);
        }

        XFlush(_display);
    }
} // namespace blade
