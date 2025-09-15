#include "window/window.h"

#ifdef BLADE_PLATFORM_WINDOWS

namespace blade
{
    std::optional<std::unique_ptr<window>> window::create(
        const std::string& name,
        width w,
        height h
    )
    {
        constexpr const char* WINDOW_CLASS_NAME = "Blade Window Class";
        std::unique_ptr<window> wnd = std::make_unique<window>(window(name, w, h));

        wnd->_hinstance = GetModuleHandle(NULL);

        HICON icon = LoadIcon(wnd->_hinstance, IDI_APPLICATION);
        WNDCLASSA wc{0};
        wc.style = CS_DBLCLKS;
        wc.lpfnWndProc = window::WindowProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = wnd->_hinstance;
        wc.hIcon = icon;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszClassName = WINDOW_CLASS_NAME;

        if (!RegisterClassA(&wc))
        {
            MessageBoxA(
                0,
                "Window registration failed",
                "Error!",
                MB_ICONEXCLAMATION | MB_OK
            );
            return std::nullopt;
        }

        constexpr uint32_t client_x = 300;
        constexpr uint32_t client_y = 100;
        const uint32_t client_width = wnd->_width;
        const uint32_t client_height = wnd->_height;

        uint32_t window_x = client_x;
        uint32_t window_y = client_y;
        uint32_t window_width = client_width;
        uint32_t window_height = client_height;

        uint32_t window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
        constexpr uint32_t window_ex_style = WS_EX_APPWINDOW;

        window_style |= WS_MAXIMIZEBOX;
        window_style |= WS_MINIMIZEBOX;
        window_style |= WS_THICKFRAME;

        RECT border_rect = {0, 0, 0, 0};
        AdjustWindowRectEx(&border_rect, window_style, FALSE, window_ex_style);

        window_x += border_rect.left;
        window_y += border_rect.top;

        // Grow by the size of the OS border
        window_width += border_rect.right - border_rect.left;
        window_height += border_rect.bottom - border_rect.top;

        wnd->_hwnd = CreateWindowEx(
            window_ex_style,
            WINDOW_CLASS_NAME,
            wnd->_title.c_str(),
            window_style,
            window_x,
            window_y,
            window_width,
            window_height,
            nullptr,
            nullptr,
            wnd->_hinstance,
            wnd.get()
        );

        wnd->_is_initialized = true;

        logger::info("Window \"{}\" initialized", name);

        return wnd;
    }

    void window::show()
    {
        if (!_is_initialized)
        {
            return;
        }

        bool should_activate = true;
        i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;

        _should_close = false;
        ShowWindow(_hwnd, show_window_command_flags);
        logger::info("Window \"{}\" showing", _title);
    }

    void window::close()
    {
        if (_should_close)
        {
            return;
        }
    }

    void window::shutdown()
    {
        if (!_is_initialized)
        {
            return;
        }

        _is_initialized = false;
        logger::info("Window \"{}\" shutting down", _title);
    }

    bool window::should_close()
    {
        _pump_messages();

        return _should_close;
    }

    void window::_pump_messages()
    {
        MSG msg;

        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        return;
    }

    bool window::set_title(const std::string& title) noexcept
    {
        BOOL result = SetWindowTextA(_hwnd, title.c_str());

        return static_cast<bool>(result);
    }

    struct gfx::framebuffer_create_info::native_window_data window::get_window_handle() const noexcept
    {
        struct gfx::framebuffer_create_info::native_window_data data = {
            .hwnd = _hwnd,
            .hinstance = _hinstance,
        };
        return data;
    }


    void window::_handle_resize(struct width width, struct height height)
    {
        _width = width.w;
        _height = height.h;
        events::dispatch(events::window_resize(width, height, *this));
    }

    //
    //  gfx::surface_info window::get_surface_info() {
    //     gfx::surface_info info {};
    //
    //     info.hInstance = _hinstance;
    //     info.hWnd = _hWnd;
    //     info.type = gfx::target_type::WINDOW;
    //
    //     return info;
    // }

    LRESULT CALLBACK window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        window* p_wnd = nullptr;
        switch (message)
        {
        case WM_NCCREATE:
            {
                CREATESTRUCT* p_create = reinterpret_cast<CREATESTRUCT*>(lParam);
                p_wnd = reinterpret_cast<window*>(p_create->lpCreateParams);
                SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(p_wnd));
            }
            break;
        case WM_ERASEBKGND: // notify the OS that erasing the screen will be handled by Application
            return 1;

        case WM_CLOSE:
            {
                p_wnd = reinterpret_cast<window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
                p_wnd->close();
                events::dispatch(events::window_close());
                return true;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            p_wnd = reinterpret_cast<window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

            if (p_wnd)
            {
                p_wnd->close();
            }
            return 0;

        case WM_SIZE:
            {
                RECT r;
                GetClientRect(hWnd, &r);
                u32 width = r.right - r.left;
                u32 height = r.bottom - r.top;
                p_wnd = reinterpret_cast<window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
                p_wnd->_handle_resize(width, height);

                // events::window_resize ev = events::window_resize({ width} , { height);
                // events::dispatch<events::window_resize>(ev);
            }
            break;

        case WM_PAINT:
            {
                PAINTSTRUCT ps{};
                HDC hdc = BeginPaint(hWnd, &ps);
                RECT client_rect{};
                GetClientRect(hWnd, &client_rect);
                FillRect(hdc, &client_rect, (HBRUSH)(COLOR_WINDOW + 1));
                EndPaint(hWnd, &ps);
            }
            break;

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
            {
                bool pressed = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN);
                // Keys key = static_cast<Keys>(wParam);

                // Pass the input subsystem
                // input_handler->process_key(key, pressed);
            }
            break;

        case WM_MOUSEMOVE:
            // Fire an event for mouse movement
            break;
        case WM_MOUSEWHEEL:
            // Fire an event for mouse movement
            break;

        case WM_LBUTTONUP:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDOWN:
            {
                // TODO: fire events for mouse buttons
            }
            break;
        }

        return DefWindowProc(hWnd, message, wParam, lParam);
    }
} // blade namespace

#endif // BLADE_PLATFORM_WINDOWS
