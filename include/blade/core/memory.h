#ifndef BLADE_CORE_MEMORY_H
#define BLADE_CORE_MEMORY_H

#include "core/types.h"

namespace blade
{
    namespace core
    {
        struct memory
        {
            void* data { nullptr };
            usize size { 0 };
        };
    } // core namespace
} // blade namespace

#endif // BLADE_CORE_MEMORY_H
