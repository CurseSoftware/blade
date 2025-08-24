#ifndef BLADE_MATH_OPERATIONS_H
#define BLADE_MATH_OPERATIONS_H

#include "math/traits.h"
#include "math/vec2.h"
#include "math/vec3.h"

namespace blade
{
    namespace math
    {
        /**
         * @brief Dot product two `vec2<T>` vectors
         * @return T the dot product result
         */
        template<typename T> requires traits::is_number<T>
        [[nodiscard]] T dot(const vec2<T>& v1, const vec2<T>& v2) noexcept
        {
            return v1[0] * v2[0] + v1[1] * v2[1];
        }

        /**
         * @brief Dot product two `vec3<T>` vectors
         * @return T the dot product result
         */
        template<typename T> requires traits::is_number<T>
        [[nodiscard]] T dot(const vec3<T>& v1, const vec3<T>& v2) noexcept
        {
            return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] + v2[2];
        }

        template<typename T> requires traits::is_number<T>
        [[nodiscard]] vec3<T> cross(const vec3<T>& a, const vec3<T>& b) noexcept
        {
            return vec3<T> {
                a[1] * b[2] - a[2] * b[1],
                a[2] * b[0] - a[0] * b[2],
                a[0] * b[1] - a[1] * b[0],
            };
        }

    } // math namespace
} // blade namespace

#endif // BLADE_MATH_OPERATIONS_H
