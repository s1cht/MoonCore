#pragma once
#include "MoonCore/Types.hpp"

namespace MC::Utility
{
    // TODO: Write a proper Int128
    struct alignas(16) Int128
    {
    public:
        Int128() noexcept = default;
        template<Concept::Integral T> Int128(T value) noexcept : High(0), Low(value) {}
        Int128(const Int128&) noexcept = default;
        Int128(Int128&&) noexcept = default;
        ~Int128() noexcept = default;

    public:
        Int128& operator=(const Int128&) noexcept = default;
        Int128& operator=(Int128&&) noexcept = default;
    public:
        u64 High;
        u64 Low;
    };
}

using i128 = MC::Utility::Int128;
using u128 = MC::Utility::Int128;