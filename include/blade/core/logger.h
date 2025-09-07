#ifndef BLADE_CORE_LOGGER_H
#define BLADE_CORE_LOGGER_H
#include <format>
#include <iostream>
#include "defines.h"

namespace blade 
{
    namespace logger 
    {
        enum text_color
        {
            red =
#ifdef BLADE_PLATFORM_LINUX
                31
#elif defined(BLADE_PLATFORM_WINDOWS)
                // windows
#endif
            , magenta =
#ifdef BLADE_PLATFORM_LINUX
                35
#elif defined(BLADE_PLATFORM_WINDOWS)
                // windows
#endif
            , blue =
#ifdef BLADE_PLATFORM_LINUX
                34
#elif defined(BLADE_PLATFORM_WINDOWS)
                // windows
#endif
            , yellow =
#ifdef BLADE_PLATFORM_LINUX
                33
#elif defined(BLADE_PLATFORM_WINDOWS)
                // windows
#endif
            , green =
#ifdef BLADE_PLATFORM_LINUX
                32
#elif defined(BLADE_PLATFORM_WINDOWS)
                // windows
#endif
            , cyan = 
#ifdef BLADE_PLATFORM_LINUX
                36
#elif defined(BLADE_PLATFORM_WINDOWS)
                // windows
#endif
            , standard = 
#ifdef BLADE_PLATFORM_LINUX
                0
#elif defined(BLADE_PLATFORM_WINDOWS)
                // windows
#endif
        };

        template<text_color C>
        struct color
        {
            friend std::ostream& operator<<(std::ostream& os, const color& c)
            {
                return os << "\033[" << C << "m";
            }
        };


        template <typename... Args>
        inline void fatal(const std::format_string<Args...> fmt, Args&&... args) 
        {
            std::cout << color<text_color::red>{} << "[FATAL] " << std::vformat(fmt.get(), std::make_format_args(args...)) << '\n' << color<text_color::standard>{};
        }

        template <typename... Args>
        inline void error(const std::format_string<Args...> fmt, Args&&... args) 
        {
            std::cout << color<text_color::red>{} << "[ERROR] " << std::vformat(fmt.get(), std::make_format_args(args...)) << '\n' << color<text_color::standard>{};
        }

        template <typename... Args>
        inline void warn(const std::format_string<Args...> fmt, Args&&... args) 
        {
            std::cout << color<text_color::yellow>{} << "[WARN]  " << std::vformat(fmt.get(), std::make_format_args(args...)) << '\n' << color<text_color::standard>{};
        }

        template <typename... Args>
        inline void debug(const std::format_string<Args...> fmt, Args&&... args) 
        {
            std::cout << color<text_color::magenta>{} << "[DEBUG] " << std::vformat(fmt.get(), std::make_format_args(args...)) << '\n' << color<text_color::standard>{};
        }

        template <typename... Args>
        inline void info(const std::format_string<Args...> fmt, Args&&... args) 
        {
            std::cout << color<text_color::green>{} << "[INFO]  " << std::vformat(fmt.get(), std::make_format_args(args...)) << '\n' << color<text_color::standard>{};
        }

        template <typename... Args>
        inline void trace(const std::format_string<Args...> fmt, Args&&... args) 
        {
            std::cout << color<text_color::cyan>{} << "[TRACE] " << std::vformat(fmt.get(), std::make_format_args(args...)) << '\n' << color<text_color::standard>{};
        }

    } // logger namespace
} // blade namespace

#endif // BLADE_CORE_LOGGER_H
