#ifndef BLADE_GFX_PROGRAM_H
#define BLADE_GFX_PROGRAM_H
#include "gfx/handle.h"

namespace blade
{
    namespace gfx
    {
        struct program
        {
            shader_handle vertex   { BLADE_NULL_HANDLE };
            shader_handle fragment { BLADE_NULL_HANDLE };
        };
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_PROGRAM_H
