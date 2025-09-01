#ifndef BLADE_CORE_TRAITS_H
#define BLADE_CORE_TRAITS_H

#include <type_traits>
#include <tuple>

namespace blade
{
    namespace core
    {
        namespace traits
        {
            template <typename T>
            concept is_reflectable = std::is_aggregate_v<T> && !std::is_polymorphic_v<T>;

            template <typename T, typename = void>
            struct member_count;
            
            
            template<typename T>
            struct member_count<T, std::void_t<decltype(std::tuple_size_v<T>)>> {
                static constexpr std::size_t value = std::tuple_size_v<T>;
            };
        } // traits namespace
    } // core namespace
} // blade namespace

#endif // BLADE_CORE_TRAITS_H
