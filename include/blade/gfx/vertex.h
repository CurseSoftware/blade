#ifndef BLADE_GFX_VERTEX_H
#define BLADE_GFX_VERTEX_H

#include "core/core.h"
#include "math/math.h"
#include "math/vec2.h"
#include "math/vec3.h"

#include <functional>
#include <optional>
#include <vector>

namespace blade
{
    namespace gfx
    {
        // TODO: add shader reflection for this
        enum class vertex_semantic : u32
        {
            position = 0,
            normal = 1,
            color = 2,
            texcoord0 = 3,
            texcoord1 = 4,
            texcoord2 = 5,
            texcoord3 = 6,
        };

        struct attribute
        {
            const char* name { nullptr };
            u32 count { 0 };

            enum class datatype : u32
            {
                uint_8 = 0,
                int_16 = 1,
                f32 = 2
            } type;

            vertex_semantic semantic { vertex_semantic::position };

            u32 offset { 0 };

            static usize datatype_to_size(datatype dt)
            {
                switch (dt)
                {
                    case datatype::f32:
                        return sizeof(float);
                    case datatype::uint_8:
                        return sizeof(u8);
                    case datatype::int_16:
                        return sizeof(i16);
                }
            }

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
                [[nodiscard]] recording& add(const char*, u32 count, attribute::datatype type, const vertex_semantic semantic) noexcept;

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

            const std::vector<attribute>& attributes() const noexcept
            {
                return _attributes;
            }

            u32 stride() const noexcept
            {
                return _stride;
            }

        private:
            void add_attribute_(attribute attrib) noexcept
            {
                attrib.offset = _stride;
                
                _stride += attribute::datatype_to_size(attrib.type) * attrib.count;
                
                _attributes.push_back(attrib);

            }

        private:
            enum class state
            {
                uninitialized,
                recording,
                finalized
            } _state { state::uninitialized };

            std::vector<attribute> _attributes {};

            usize _attribute_index { 0 };
            u32 _stride { 0 };

            recording _recording { *this };
        };
    } // gfx namespace
} // blade namespace

#endif // BLADE_GFX_VERTEX_H
