#ifndef BLADE_CORE_TYPES_H
#define BLADE_CORE_TYPES_H

// Disable warning for cstdint with MSVC compiler
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4103)
#endif // _MSC_VER

#include <cstdint>

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

namespace blade
{
    using u8 = uint8_t;
    using u16 = uint16_t;   
    using u32 = uint32_t;
    using u64 = uint64_t;

    using i8 = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;

    using f32 = float;
    using f64 = double;

    using usize = std::size_t;

    /// @brief Struct type for width 
    struct width
    {
        width(u32 w): w(w) {}

        u32 w {0};
    };
   
    /// @brief Struct type for height
    struct height 
    {
        height(u32 h): h(h) {}

        u32 h {0};
    };
}

#endif
