#ifndef BLADE_MATH_VEC2_H
#define BLADE_MATH_VEC2_H

#include <array>
#include <stdexcept>
#include "math/traits.h"

namespace blade
{
    namespace math
    {
        template<typename T> requires traits::is_number<T>
        struct vec2
        {
            [[nodiscard]] vec2() noexcept
                : _data{ 0.f, 0.f }
            {}

            [[nodiscard]] vec2(T v1, T v2) noexcept
                : _data{ v1, v2 }
            {}


            [[nodiscard]] vec2(T arr[2]) noexcept 
                : _data{ arr }
            {}

            vec2(const vec2& other) noexcept
            {
                _data[0] = other._data[0];
                _data[1] = other._data[1];
            }

            constexpr vec2& operator=(T arr[2]) noexcept
            {
                _data = arr;

                return *this;
            }

            vec2& operator=(const vec2& other) noexcept
            {
                _data[0] = other._data[0];
                _data[1] = other._data[1];
                
                return *this;
            }

            vec2& operator=(vec2&& other) noexcept
            {
                if (this != &other)
                {
                    _data[0] = other._data[0];
                    _data[1] = other._data[1];
                }

                return *this;
            }

            [[nodiscard]] vec2<T> operator+(const vec2<T>& other) const noexcept
            {
                return vec2<T>(_data[0] + other._data[0], _data[1] + other._data[1]);
            }

            [[nodiscard]] T operator[](std::size_t idx) const
            {
                if (idx > 1)
                {
                    throw std::out_of_range("Accessing blade::gfx::vec2 with index greater than 1");
                }

                return _data[idx];
            }

            [[nodiscard]] T magnitude() const noexcept
            {
                return _data[0] * _data[0] + _data[1] * _data[1];
            }

            [[nodiscard]] std::string to_string() const noexcept
            {
                return "(" + std::to_string(_data[0]) + "," + std::to_string(_data[1]) + ")";
            }

            T _data[2] { 0.f, 0.f };
        };

    } // math namespace
} // blade namespace

#endif // BLADE_MATH_VEC2_H
