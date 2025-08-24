#ifndef BLADE_GFX_VERTEX_H
#define BLADE_GFX_VERTEX_H

#include "core/core.h"
#include "math/math.h"
#include "math/vec2.h"
#include "math/vec3.h"

namespace blade
{
    namespace gfx
    {
        struct vertex
        {
            math::vec2<f32> position;
            // f32 uv_x;
            math::vec3<f32> color;
            // f32 uv_y;
        };
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VERTEX_H
