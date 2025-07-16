#ifndef BLADE_GFX_HANDLE_H
#define BLADE_GFX_HANDLE_H

#include "core/core.h"
#include <limits>

namespace blade
{
    namespace gfx
    {
        const u16 BLADE_NULL_HANDLE = std::numeric_limits<u16>::max();
    } // gfx namespace
} // blade namespace
#define MAKE_BLADE_HANDLE(_handle_name)                                                         \
    namespace blade::gfx                                                                        \
    {                                                                                           \
        struct _handle_name {                                                                   \
            u16 index = BLADE_NULL_HANDLE;                                                      \
            bool operator>(const _handle_name& h) const { return index > h.index; }             \
            bool operator==(const _handle_name& h) const { return index == h.index; }           \
        };                                                                                      \
    }                                                                                           \
    namespace std                                                                               \
    {                                                                                           \
        template <>                                                                             \
        struct hash<::blade::gfx::_handle_name>                                                 \
        {                                                                                       \
            std::size_t operator()(const ::blade::gfx::_handle_name h) const                    \
            {                                                                                   \
                return std::hash<blade::u16>{}(h.index);                                        \
            }                                                                                   \
        };                                                                                      \
    }                                                                                           


MAKE_BLADE_HANDLE(framebuffer_handle);
MAKE_BLADE_HANDLE(view_handle);

#undef MAKE_BLADE_HANDLE

#endif // BLADE_GFX_HANDLE_H
