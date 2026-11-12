#pragma once
#include "MoonCore/Core.hpp"

namespace MC::Concept
{
    template <typename D, typename T>
    concept Deleter = requires(T* pointer)
    {
        { D::Delete(pointer) } -> SameAs<void>;
    };
}

namespace MC::Memory
{
    template <typename T>
    struct DefaultDeleter
    {
        static void Delete(T* pointer) noexcept
        {
            delete pointer;
        }
    };
}