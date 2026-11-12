#pragma once
#include "MoonCore/Types.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "String/StringShared.hpp"
#include "MoonCore/Utility/Algorithm.hpp"

namespace MC
{
    template<usize BitCount>
    class Bitset
    {
    public:
        struct Reference
        {
            constexpr Reference(uptr& word, u32 bit) : Word(word), Bit(bit) {}
            
            Reference& operator=(bool state) noexcept
            {
                Word = (Word & ~(static_cast<uptr>(1) << Bit)) | (static_cast<uptr>(state) << Bit);
                return *this;
            }

            operator bool() const noexcept
            {
                return (Word & static_cast<uptr>(1) << Bit) != 0;
            }

            uptr& Word;
            u32 Bit;
        };

    public:
        static constexpr usize BitsPerWord = sizeof(uptr) * 8;
        static constexpr usize WordCount = BitCount == 0 ? 0 : (BitCount + BitsPerWord - 1) / BitsPerWord;

    public:
        constexpr Bitset() noexcept = default;
        constexpr Bitset(u64 value) noexcept
        {
            if constexpr (WordCount >= 1)
                m_Words[0] = static_cast<uptr>(value);
            if constexpr (WordCount > 1)
                m_Words[1] = static_cast<uptr>(value >> BitsPerWord);
        }
        template <Concept::Character CharacterType>
        constexpr Bitset(const CharacterType* str, usize size = ~0ull,
            CharacterType zero = CharacterType('0'), CharacterType one = CharacterType('1'))
        {
            if (size == ~0ull) size = GetStringSize(str);
            if constexpr (WordCount == 1) size = Min(size, BitCount);

            for (usize i = 0; i < size; ++i)
            {
                MC_CHECK(str[i] == zero || str[i] == one, "Character != zero nor one");
                if (str[i] == zero) continue;
                Set(BitCount - size + i, true);
            }
        }

    public:
        static constexpr usize Size() noexcept { return BitCount; }

    public:
        constexpr Reference operator[](usize bitIndex) noexcept
        {
            MC_CHECK(bitIndex < BitCount, "bitIndex >= BitCount");
            return Reference{ m_Words[GetWord(bitIndex)], static_cast<u32>(bitIndex % BitsPerWord) };
        }

        constexpr bool operator[](usize bitIndex) const noexcept
        {
            MC_CHECK(bitIndex < BitCount, "bitIndex >= BitCount");
            return Get(GetWord(bitIndex), static_cast<u32>(bitIndex % BitsPerWord));
        }

        template<usize OtherBitCount>
        constexpr bool operator==(const Bitset<OtherBitCount>& other) const noexcept
        {
            if constexpr (BitCount != OtherBitCount) return false;
            for (usize i = 0; i < WordCount; ++i)
                if (m_Words[i] != other.m_Words[i]) return false;
            return true;
        }

        constexpr Bitset& operator&=(const Bitset& other) noexcept
        {
            for (usize i = 0; i < WordCount; i++)
                m_Words[i] &= other.m_Words[i];
            return *this;
        }

        constexpr Bitset& operator|=(const Bitset& other) noexcept
        {
            for (usize i = 0; i < WordCount; i++)
                m_Words[i] |= other.m_Words[i];
            return *this;
        }

        constexpr Bitset& operator^=(const Bitset& other) noexcept
        {
            for (usize i = 0; i < WordCount; i++)
                m_Words[i] ^= other.m_Words[i];
            return *this;
        }

        constexpr Bitset operator~() const noexcept;

        constexpr Bitset& operator>>=(usize position) noexcept
        {
            const auto wordshift = static_cast<uptr>(position / BitsPerWord);
            if (wordshift != 0)
                for (uptr i = 0; i <= WordCount; ++i)
                    m_Words[i] = wordshift <= WordCount - i ? m_Words[i + wordshift] : 0;

            if ((position %= BitsPerWord) != 0)
            {
                for (uptr i = 0; i < WordCount; ++i)
                    m_Words[i] = (m_Words[i] >> position) | (m_Words[i + 1] << (BitsPerWord - position));
                m_Words[WordCount] >>= position;
            }
            return *this;
        }

        constexpr Bitset& operator>>(usize position) noexcept
        {
            Bitset result = *this;
            return result >>= position;
        }

        constexpr Bitset& operator<<=(usize position) noexcept
        {
            const auto wordshift = static_cast<uptr>(position / BitsPerWord);
            if (wordshift != 0)
                for (uptr i = WordCount; 0 <= i; --i)
                    m_Words[i] = wordshift <= i ? m_Words[i - wordshift] : 0;

            if ((position %= BitsPerWord) != 0) 
            {
                for (uptr i = WordCount; 0 < i; --i)
                    m_Words[i] = (m_Words[i] << position) | (m_Words[i - 1] >> (BitsPerWord - position));

                m_Words[0] <<= position;
            }
            Trim();
            return *this;
        }

        constexpr Bitset operator<<(usize position) noexcept
        {
            Bitset result = *this;
            return result <<= position;
        }

    public:
        constexpr bool Test(usize bitIndex) const noexcept
        {
            MC_CHECK(bitIndex < BitCount, "bitIndex >= BitCount");
            return Get(GetWord(bitIndex), static_cast<u32>(bitIndex % BitsPerWord));
        }

        constexpr bool All() const noexcept
        {
            for (usize currentWord = 0; currentWord < WordCount; ++currentWord)
            {
                if (currentWord == WordCount - 1)
                {
                    uptr mask = NumericLimit<uptr>::Max >> (WordCount * BitsPerWord - BitCount);
                    return (m_Words[currentWord] & mask) == mask;
                }
                if (m_Words[currentWord] != NumericLimit<uptr>::Max) return false;
            }
            return true;
        }

        constexpr bool Any() const noexcept
        {
            for (usize currentWord = 0; currentWord < WordCount; ++currentWord)
                if (m_Words[currentWord] != 0)
                    return true;
            return false;
        }

        constexpr bool None() const noexcept
        {
            for (usize currentWord = 0; currentWord < WordCount; ++currentWord)
                if (m_Words[currentWord] != 0)
                    return false;
            return true;
        }

        constexpr usize Count() const noexcept
        {
            usize result = 0;
            for (usize i = 0; i < WordCount; ++i)
            {
                uptr word = m_Words[i];
                for (usize chunk = 0; chunk < BitsPerWord; chunk += 16)
                    result += PopCount(static_cast<u16>(word >> chunk));
            }
            return result;
        }

        constexpr Bitset& Set() noexcept
        {
            if constexpr (WordCount == 0) return *this;
            for (usize i = 0; i < WordCount - 1; ++i) m_Words[i] = NumericLimit<uptr>::Max;
            m_Words[WordCount - 1] |= NumericLimit<uptr>::Max>>(WordCount * BitsPerWord - BitCount);
            return *this;
        }

        constexpr Bitset& Set(usize bitIndex, bool state = true) noexcept
        {
            MC_CHECK(bitIndex < BitCount, "bitIndex >= BitCount");
            Set(GetWord(bitIndex), bitIndex % BitsPerWord, state);
            return *this;
        }

        constexpr Bitset& Reset() noexcept
        {
            for (usize i = 0; i < WordCount; ++i) m_Words[i] = 0;
            return *this;
        }

        constexpr Bitset& Reset(usize bitIndex) noexcept
        {
            MC_CHECK(bitIndex < BitCount, "bitIndex >= BitCount");
            Set(GetWord(bitIndex), bitIndex % BitsPerWord, false);
            return *this;
        }

        constexpr Bitset& Flip() noexcept
        {
            if constexpr (WordCount == 0) return *this;
            for (usize i = 0; i < WordCount - 1; ++i) m_Words[i] = ~m_Words[i];
            m_Words[WordCount - 1] = ~m_Words[WordCount - 1] & NumericLimit<uptr>::Max >> WordCount * BitsPerWord - BitCount;
            return *this;
        }

        constexpr Bitset& Flip(usize bitIndex) noexcept
        {
            MC_CHECK(bitIndex < BitCount, "bitIndex >= BitCount");
            Set(GetWord(bitIndex), bitIndex % BitsPerWord, !Test(bitIndex));
            return *this;
        }

    private:
        constexpr void Set(usize word, usize bit, bool state) noexcept
        {
            m_Words[word] = (m_Words[word] & ~(static_cast<uptr>(1) << bit)) | (static_cast<uptr>(state) << bit);
        }

        constexpr bool Get(usize word, usize bit) const noexcept
        {
            return (m_Words[word] & static_cast<uptr>(1) << bit) != 0;
        }

        static constexpr usize GetWord(usize bitIndex) noexcept
        {
            return bitIndex / BitsPerWord;
        }

    private:
        static constexpr usize PopCount(u16 x) noexcept
        {
            x = (x & 0x5555u) + ((x >> 1) & 0x5555u);
            x = (x & 0x3333u) + ((x >> 2) & 0x3333u);
            x = (x & 0x0F0Fu) + ((x >> 4) & 0x0F0Fu);
            x = (x & 0x00FFu) + ((x >> 8) & 0x00FFu);
            return x;
        }

        constexpr void Trim() noexcept
        {
            if constexpr (BitCount != 0 && BitCount % BitsPerWord == 0) 
                return;
            m_Words[WordCount] &= (1 << BitCount % BitsPerWord) - 1;
        }

    private:
        uptr m_Words[WordCount == 0 ? 1 : WordCount]{};
    };

    template <usize BitCount>
    constexpr Bitset<BitCount> Bitset<BitCount>::operator~() const noexcept
    {
        Bitset copy = *this;
        copy.Flip();
        return copy;
    }
}

template<usize BitCount>
MC::Bitset<BitCount> operator&(const MC::Bitset<BitCount>& left,
                                         const MC::Bitset<BitCount>& right) noexcept
{
    MC::Bitset<BitCount> result = left;
    return result &= right;
}

template<usize BitCount>
MC::Bitset<BitCount> operator|(const MC::Bitset<BitCount>& left,
                                         const MC::Bitset<BitCount>& right) noexcept
{
    MC::Bitset<BitCount> result = left;
    return result |= right;
}

template<usize BitCount>
MC::Bitset<BitCount> operator^(const MC::Bitset<BitCount>& left,
                                         const MC::Bitset<BitCount>& right) noexcept
{
    MC::Bitset<BitCount> result = left;
    return result ^= right;
}