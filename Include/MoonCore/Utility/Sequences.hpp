#pragma once
#include "MoonCore/TypeTraits.hpp"

namespace MC::Utility
{
    template<typename T, T... Sequence>
        requires IsIntegral<T>
    struct IntSequence
    {
        using ValueType = T;
        static constexpr usize Size() noexcept { return sizeof...(Sequence); }
    };

    template<typename T, T Count> using MakeIntSequence = __make_integer_seq<IntSequence, T, Count>;

    template<usize... Indices> using IndexSequence = IntSequence<usize, Indices...>;
    template<usize N> using MakeIndexSequence = MakeIntSequence<usize, N>;
}