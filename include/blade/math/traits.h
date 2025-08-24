#ifndef BLADE_MATH_UTILS_H
#define BLADE_MATH_UTILS_H

#include <type_traits>
namespace blade
{
    namespace math
    {
        namespace traits
        {
            template<typename T>
            concept is_number = std::is_integral_v<T> || std::is_floating_point_v<T>;
        }
    } // math namespace
} // blade namespace

#endif // BLADE_MATH_UTILS_H
