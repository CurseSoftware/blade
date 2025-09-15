#ifndef BLADE_WINDOW_WINDOW_H
#define BLADE_WINDOW_WINDOW_H

#include "core/core.h"
#include "gfx/view.h"
#include <memory>
#include <optional>

#if defined(BLADE_PLATFORM_WINDOWS)
#define NOMINMAX
#include <windows.h>
#elif defined(BLADE_PLATFORM_LINUX)
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#endif // Platform Detection

namespace blade
{
    constexpr i32 INVALID_WINDOW_ID = -1;

    class window
    {
    public:
        /// @brief Create a new window instance
        /// @param name Title of the window
        /// @param width Width of the window
        /// @param height Height of the window
        /// @return Some(window) if successful, std::nullopt otherwise
        static std::optional<std::unique_ptr<window>> create(const std::string& name, width w, height h);

        /// @brief Open and display the window
        void show();

        /// @brief Set the title of the window
        /// @param title Title to change window title to
        /// @return `true` if successful `false` otherwise
        bool set_title(const std::string& title) noexcept;

        /// @brief close the window
        void close();

        /// @brief Whether the window should close or not
        /// @return `true` if window should close and `false` otherwise
        bool should_close();

        /// @brief Shutdown the window
        void shutdown();

        std::optional<std::unique_ptr<window>> spawn_child(const std::string& name, width w, height h);

        /// @brief Getter for the title of the window
        /// @return Const reference to the title
        const std::string& get_title() const { return _title; }

        /// @brief Getter for the id of the window
        /// @return i32 of the window id
        i32 get_id() const { return _id; }

        /// @brief Get the window handle for this window
        /// @return data for the window handle. See gfx module
        struct gfx::framebuffer_create_info::native_window_data get_window_handle() const noexcept;

        /// @brief Getter for the current width of the window
        /// @return u32 of the window width
        u32 get_width() const noexcept { return _width; }

        /// @brief Getter for the current height of the window
        /// @return u32 of the window height
        u32 get_height() const noexcept { return _height; }

    private:
        [[nodiscard]] window(
            const std::string& name
            , width width
            , height height
        )
            : _width(width.w)
              , _height(height.h)
              , _title(name)
        {
        }

        /// @brief Pump pending window messages
        void _pump_messages();

        /// @brief Handle resizing for this window
        /// @param width Width after resize
        /// @param height Height after resize
        void _handle_resize(struct width width, struct height height);

        i32 _id{INVALID_WINDOW_ID};
        u32 _width{0};
        u32 _height{0};
        std::string _title{""};
        bool _is_initialized{false};
        bool _should_close{true};

#if defined(BLADE_PLATFORM_WINDOWS)
        HWND _hwnd;
        HINSTANCE _hinstance;

        /// @brief Callback function for handling window events
        static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
#elif defined(BLADE_PLATFORM_LINUX)
        Display* _display;
        Window _window;
        Atom _wm_delete_window;
        static XContext _global_x11_context;

        /// @brief Callback function for handling X window events
        static void _handle_x11_event(XEvent& event);
#endif // Platform Detection
    };
} // namespace blade

#endif // BLADE_WINDOW_WINDOW_H
