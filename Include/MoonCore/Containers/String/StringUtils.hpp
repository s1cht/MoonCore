#pragma once
#include "MoonCore/Core.hpp"
#include "String.hpp"
#include "StringView.hpp"
#include "MoonCore/Utility/Pair.hpp"

namespace MC::Utility
{
    namespace Details
    {
        constexpr char8 HexValues[16] = {
            TEXT('0'), TEXT('1'), TEXT('2'), TEXT('3'), TEXT('4'), TEXT('5'), TEXT('6'), TEXT('7'),
            TEXT('8'), TEXT('9'), TEXT('A'), TEXT('B'), TEXT('C'), TEXT('D'), TEXT('E'), TEXT('F')
        };

        constexpr Pair<char8, char8> NumberToHex(u8 number) noexcept
        {
            return { HexValues[number >> 4], HexValues[number & 0x0F] };
        }
    }

    template<Concept::UnsignedIntegral T>
    static MC::String ConvertNumberToHex(T number) noexcept
    {
        String result = TEXT("0000000000000000");

        const usize byteCount = sizeof(T) / 8;

        for (usize i = 0; i < byteCount; ++i)
        {
            Pair hex = Details::NumberToHex(static_cast<u8>(number >> ((sizeof(T) - 8) - (i * 8)) & 0xFF));
            result[i * 2] = hex.Key;
            result[i * 2 + 1] = hex.Value;
        }

        return result;
    }

    template<Concept::SignedIntegral T>
    MC::String ConvertNumberToHex(T number) noexcept
    {
        using Unsigned = MC::Details::MakeUnsigned<T>;
        return ConvertNumberToHex<Unsigned>(static_cast<Unsigned>(number));
    }

    static constexpr u64 ConvertHexToNumber(const MC::StringView& hex) noexcept
    {
        u64 result = 0;
        for (usize i = 0; i < hex.Size(); ++i)
        {
            u8 value;
            char8 c = hex[i];
            if (c >= TEXT('0') && c <= TEXT('9')) value = c - '0';
            else if (c >= TEXT('A') && c <= TEXT('F')) value = 10 + (c - 'A');
            else if (c >= TEXT('a') && c <= TEXT('f')) value = 10 + (c - 'a');
            else return ~0ull;
            result = (result << 4) | value;
        }

        return result;
    }
}
