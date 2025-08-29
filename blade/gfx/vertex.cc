#include "gfx/vertex.h"

namespace blade 
{
    namespace gfx
    {
        
        vertex_layout& vertex_layout::recording::end() const noexcept
        {
            _layout._state = state::finalized;
            return _layout;
        }

        vertex_layout::recording& vertex_layout::recording::add(const char* name, u32 count, attribute::datatype type, const vertex_semantic semantic) noexcept
        {
            _layout.add_attribute_({
                name,
                count,
                type,
                semantic
            });

            return *this;
        }

    } // gfx namespace
} // gfx namespace
