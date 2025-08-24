#ifndef BLADE_MATH_VEC3_H
#define BLADE_MATH_VEC3_H

#include <array>
#include <stdexcept>
#include <string>
#include "math/traits.h"

namespace blade
{
    namespace math
    {
        template<typename T> requires traits::is_number<T>
        struct vec3
        {
            [[nodiscard]] vec3() noexcept 
                : _data{ 0.f, 0.f, 0.f } 
            {}
            
            [[nodiscard]] vec3(T v1, T v2, T v3) noexcept
                : _data{ v1, v2, v3 }
            {}


            [[nodiscard]] vec3(T arr[3]) noexcept 
                : _data{ arr }
            {}

            [[nodiscard]] std::string to_string() const noexcept
            {
                return "(" + std::to_string(_data[0]) + "," + std::to_string(_data[1]) + "," + std::to_string(_data[2]) + ")";
            }

            vec3(const vec3& other) noexcept
            {
                _data[0] = other._data[0];
                _data[0] = other._data[0];
                _data[2] = other._data[2];
            }

            constexpr vec3& operator=(T arr[2]) noexcept
            {
                _data = arr;

                return *this;
            }

            vec3& operator=(const vec3& other) noexcept
            {
                _data[0] = other._data[0];
                _data[0] = other._data[0];
                _data[2] = other._data[2];
                
                return *this;
            }

            vec3& operator=(vec3&& other) noexcept
            {
                if (this != &other)
                {
                    _data[0] = other._data[0];
                    _data[0] = other._data[0];
                    _data[2] = other._data[2];
                }

                return *this;
            }

            [[nodiscard]] T operator[](std::size_t idx) const
            {
                if (idx > 2)
                {
                    throw std::out_of_range("Accessing blade::gfx::vec2 with index greater than 1");
                }

                return _data[idx];
            }

            [[nodiscard]] T magnitude() const noexcept
            {
                return _data[0] * _data[0] + _data[1] + _data[1] + _data[2] * _data[2];
            }

            T _data[3] { 0.f, 0.f, 0.f };
        };

    } // math namespace
} // blade namespace

#endif // BLADE_MATH_VEC3_H
