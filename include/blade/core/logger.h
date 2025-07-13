#ifndef BLADE_CORE_LOGGER_H
#define BLADE_CORE_LOGGER_H
#include <format>
#include <iostream>

namespace blade 
{
    namespace logger 
    {
        template <typename... Args>
        inline void fatal(const std::format_string<Args...> fmt, Args&&... args) 
        {
            std::cout << "[FATAL] " << std::vformat(fmt.get(), std::make_format_args(args...)) << '\n';
        }

        template <typename... Args>
        inline void error(const std::format_string<Args...> fmt, Args&&... args) 
        {
            std::cout << "[ERROR] " << std::vformat(fmt.get(), std::make_format_args(args...)) << '\n';
        }

        template <typename... Args>
        inline void warn(const std::format_string<Args...> fmt, Args&&... args) 
        {
            std::cout << "[WARN]  " << std::vformat(fmt.get(), std::make_format_args(args...)) << '\n';
        }

        template <typename... Args>
        inline void debug(const std::format_string<Args...> fmt, Args&&... args) 
        {
            std::cout << "[DEBUG] " << std::vformat(fmt.get(), std::make_format_args(args...)) << '\n';
        }

        template <typename... Args>
        inline void info(const std::format_string<Args...> fmt, Args&&... args) 
        {
            std::cout << "[INFO]  " << std::vformat(fmt.get(), std::make_format_args(args...)) << '\n';
        }

        template <typename... Args>
        inline void trace(const std::format_string<Args...> fmt, Args&&... args) 
        {
            std::cout << "[TRACE] " << std::vformat(fmt.get(), std::make_format_args(args...)) << '\n';
        }

    } // logger namespace
} // blade namespace

#endif // BLADE_CORE_LOGGER_H
