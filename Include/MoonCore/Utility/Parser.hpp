#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Containers/String.hpp"

namespace MC::Utility
{
    class Parser
    {
    public:
        static bool Parameter(const MC::StringView& line, const MC::StringView& match) noexcept;

        static bool Value(const MC::StringView& line, const MC::StringView& match, u64* value) noexcept;
        static bool Value(const MC::StringView& line, const MC::StringView& match, u32* value) noexcept;
        static bool Value(const MC::StringView& line, const MC::StringView& match, u16* value) noexcept;
        static bool Value(const MC::StringView& line, const MC::StringView& match, u8* value) noexcept;
        static bool Value(const MC::StringView& line, const MC::StringView& match, i64* value) noexcept;
        static bool Value(const MC::StringView& line, const MC::StringView& match, i32* value) noexcept;
        static bool Value(const MC::StringView& line, const MC::StringView& match, i16* value) noexcept;
        static bool Value(const MC::StringView& line, const MC::StringView& match, i8* value) noexcept;
        static bool Value(const MC::StringView& line, const MC::StringView& match, f64* value) noexcept;
        static bool Value(const MC::StringView& line, const MC::StringView& match, f32* value) noexcept;
        static bool Value(const MC::StringView& line, const MC::StringView& match, const char8** value, usize* size) noexcept;

        static bool Line(const MC::StringView& line, const char8** result, usize* size) noexcept;
        static bool Line(const MC::StringView& line, char8* result, usize resultMaxSize) noexcept;
        static bool Line(const MC::StringView& line, MC::String* result) noexcept;
    };
}