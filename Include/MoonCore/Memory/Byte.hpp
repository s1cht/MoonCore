#pragma once
#include "MoonCore/Containers/Span.hpp"

namespace MC::Memory
{
    using Byte = u8;
    using ByteView = Span<const u8>;
    using ByteSpan = Span<u8>;

    template <typename T> 
        requires IsTriviallyCopyable<T>
    ByteView AsBytes(const T& value) noexcept {
        return { reinterpret_cast<const Byte*>(&value), sizeof(T) };
    }

    template <typename T> 
        requires IsTriviallyCopyable<T>
    ByteView AsBytes(Span<const T> values) noexcept {
        return { reinterpret_cast<const Byte*>(values.Data()), values.Size() * sizeof(T) };
    }
}