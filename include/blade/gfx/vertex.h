#ifndef BLADE_GFX_VERTEX_H
#define BLADE_GFX_VERTEX_H

#include "core/core.h"
#include "math/math.h"
#include "math/vec2.h"
#include "math/vec3.h"

#include <functional>
#include <optional>

namespace blade
{
    namespace gfx
    {
        struct attribute
        {
            const char* name { nullptr };
            u32 count { 0 };

            enum class datatype
            {
                f32,
                uint_8,
                uint_16,
                uint_32,
                int_8,
                int_16,
                int_32
            } type;

            void print() { logger::debug("Attribute \"{}\" count: {}", name, count); }
        };


        struct vertex_layout
        {
            constexpr static usize MAX_ATTRIBUTE_COUNT = 14;

            struct recording
            {
                recording(struct vertex_layout& layout) noexcept 
                    : _layout{ layout } 
                {}

                vertex_layout& end() const noexcept;
                [[nodiscard]] recording& add(const char*, u32 count, attribute::datatype type) noexcept;

            private:
                vertex_layout& _layout;
            };

            std::optional<std::reference_wrapper<recording>> begin()
            {
                if (_state != state::uninitialized) 
                {
                    return std::nullopt;
                }

                _state = state::recording;
                
                return _recording;
            }

            void print()
            {
                _attributes[0].print();
            }

        private:
            void add_attribute_(const attribute attrib) noexcept
            {
                _attributes[_attribute_index] = attrib;
            }

        private:
            enum class state
            {
                uninitialized,
                recording,
                finalized
            } _state { state::uninitialized };

            attribute _attributes[MAX_ATTRIBUTE_COUNT] {};

            usize _attribute_index { 0 };

            recording _recording { *this };
        };
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VERTEX_H
