#ifndef BLADE_CORE_DEFINES_H
#define BLADE_CORE_DEFINES_H

// Platform Detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define BLADE_PLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64 bit is required for windows"
#endif
#elif defined(__linux__) || defined(__gnu_linux__)
#define BLADE_PLATFORM_LINUX 1
#elif defined(__unix__)
#define BLADE_PLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
#define BLADE_PLATFORM_POSIX 1
#elif __APPLE__
#include <TargetConditionals.h>
#define BLADE_PLATFORM_APPLE 1
#endif

#ifdef BLADE_EXPORT
#ifdef _MSC_VER
#define BLADE_API __declspec(dllexport)
#else
#define BLADE_API __attribute__((visibility("default")))
#endif
#else
// Imports
#ifdef _MSC_VER
#define BLADE_API __declspec(dllimport)
#else
#define BLADE_API
#endif
#endif

#endif // BLADE_CORE_DEFINES_H
