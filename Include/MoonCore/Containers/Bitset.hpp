#pragma once
#include "MoonCore/Types.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "MoonCore/Containers/String/StringShared.hpp"
#include "MoonCore/Utility/Algorithm.hpp"

/**
 * @file Bitset.hpp
 * @brief Fixed-size packed boolean bitset container.
 * @ingroup bit_containers
 */

namespace MC
{
    /**
     * @class Bitset
     * @brief Compact fixed-size sequence of N bits packed into machine words.
     *
     * `Bitset<BitCount>` provides an efficient representation of fixed-size bit sequences,
     * packing 8 bits into each byte (using machine word `uptr` chunks). It provides full bitwise
     * arithmetic operators (`&`, `|`, `^`, `~`, `<<`, `>>`), individual bit query and mutation
     * via reference proxy (`Bitset::Reference`), and set inspection operations (`All()`, `Any()`,
     * `None()`, `Count()`).
     *
     * @tparam BitCount The compile-time number of bits in the bitset.
     *
     * @code
     * MC::Bitset<16> mask(0b1010'0101);
     * mask.Set(2);
     * mask[3] = true;
     * bool isSet = mask.Test(2);
     * usize totalOnBits = mask.Count();
     * @endcode
     */
    template<usize BitCount>
    class Bitset
    {
    public:
        /**
         * @struct Reference
         * @brief Proxy object representing a single modifiable bit inside a `Bitset`.
         */
        struct Reference
        {
            /**
             * @brief Constructs a reference proxy to a specific bit in a word.
             * @param word Reference to machine word containing the bit.
             * @param bit Bit index within the word.
             */
            constexpr Reference(uptr& word, u32 bit) : Word(word), Bit(bit) {}
            
            /**
             * @brief Assigns a boolean state to the referenced bit.
             * @param state Boolean value.
             * @return Reference to this proxy.
             */
            Reference& operator=(bool state) noexcept
            {
                Word = (Word & ~(static_cast<uptr>(1) << Bit)) | (static_cast<uptr>(state) << Bit);
                return *this;
            }

            /**
             * @brief Converts the referenced bit to a boolean value.
             * @return True if bit is 1, false if 0.
             */
            operator bool() const noexcept
            {
                return (Word & static_cast<uptr>(1) << Bit) != 0;
            }

            uptr& Word; ///< Reference to containing machine word.
            u32 Bit;    ///< Bit position within word.
        };

    public:
        static constexpr usize BitsPerWord = sizeof(uptr) * 8;                                 ///< Bits in single machine word.
        static constexpr usize WordCount = BitCount == 0 ? 0 : (BitCount + BitsPerWord - 1) / BitsPerWord; ///< Total words required.

    public:
        /** @brief Default constructor. Initializes all bits to zero. */
        constexpr Bitset() noexcept = default;

        /**
         * @brief Constructs a bitset from an integer value.
         * @param value 64-bit integer representing initial bit pattern.
         */
        constexpr Bitset(u64 value) noexcept
        {
            if constexpr (WordCount >= 1)
                m_Words[0] = static_cast<uptr>(value);
            if constexpr (WordCount > 1)
                m_Words[1] = static_cast<uptr>(value >> BitsPerWord);
        }

        /**
         * @brief Constructs a bitset from a string of character digits.
         * @tparam CharacterType Character type satisfying `Concept::Character`.
         * @param str Null-terminated character string.
         * @param size String length, or `~0ull` to auto-detect.
         * @param zero Character representing 0 (default `'0'`).
         * @param one Character representing 1 (default `'1'`).
         */
        template <Concept::Character CharacterType>
        constexpr Bitset(const CharacterType* str, usize size = ~0ull,
            CharacterType zero = CharacterType('0'), CharacterType one = CharacterType('1'))
        {
            if (size == ~0ull) size = GetStringSize(str);
            if constexpr (WordCount == 1) size = Min(size, BitCount);

            for (usize i = 0; i < size; ++i)
            {
                MOON_CHECK(str[i] == zero || str[i] == one, "Character != zero nor one");
                if (str[i] == zero) continue;
                Set(BitCount - size + i, true);
            }
        }

    public:
        /** @brief Returns compile-time total number of bits. */
        static constexpr usize Size() noexcept { return BitCount; }

    public:
        /**
         * @brief Accesses bit at @p bitIndex returning a modifiable proxy reference.
         * @param bitIndex 0-based bit index.
         * @return Modifiable proxy `Reference`.
         */
        constexpr Reference operator[](usize bitIndex) noexcept
        {
            MOON_CHECK(bitIndex < BitCount, "bitIndex >= BitCount");
            return Reference{ m_Words[GetWord(bitIndex)], static_cast<u32>(bitIndex % BitsPerWord) };
        }

        /**
         * @brief Accesses bit at @p bitIndex (const overload).
         * @param bitIndex 0-based bit index.
         * @return Boolean value of bit.
         */
        constexpr bool operator[](usize bitIndex) const noexcept
        {
            MOON_CHECK(bitIndex < BitCount, "bitIndex >= BitCount");
            return Get(GetWord(bitIndex), static_cast<u32>(bitIndex % BitsPerWord));
        }

        /**
         * @brief Equality comparison between two bitsets.
         * @tparam OtherBitCount Bit count of other bitset.
         * @param other Bitset to compare.
         * @return True if bit patterns are identical.
         */
        template<usize OtherBitCount>
        constexpr bool operator==(const Bitset<OtherBitCount>& other) const noexcept
        {
            if constexpr (BitCount != OtherBitCount) return false;
            for (usize i = 0; i < WordCount; ++i)
                if (m_Words[i] != other.m_Words[i]) return false;
            return true;
        }

        /** @brief Bitwise AND assignment operator. */
        constexpr Bitset& operator&=(const Bitset& other) noexcept
        {
            for (usize i = 0; i < WordCount; i++)
                m_Words[i] &= other.m_Words[i];
            return *this;
        }

        /** @brief Bitwise OR assignment operator. */
        constexpr Bitset& operator|=(const Bitset& other) noexcept
        {
            for (usize i = 0; i < WordCount; i++)
                m_Words[i] |= other.m_Words[i];
            return *this;
        }

        /** @brief Bitwise XOR assignment operator. */
        constexpr Bitset& operator^=(const Bitset& other) noexcept
        {
            for (usize i = 0; i < WordCount; i++)
                m_Words[i] ^= other.m_Words[i];
            return *this;
        }

        /** @brief Bitwise NOT operator. Inverts all bits. */
        constexpr Bitset operator~() const noexcept;

        /** @brief Bitwise right shift assignment operator. */
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

        /** @brief Bitwise right shift operator. */
        constexpr Bitset& operator>>(usize position) noexcept
        {
            Bitset result = *this;
            return result >>= position;
        }

        /** @brief Bitwise left shift assignment operator. */
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

        /** @brief Bitwise left shift operator. */
        constexpr Bitset operator<<(usize position) noexcept
        {
            Bitset result = *this;
            return result <<= position;
        }

    public:
        /**
         * @brief Tests value of bit at @p bitIndex.
         * @param bitIndex 0-based bit index.
         * @return True if set, false if clear.
         */
        constexpr bool Test(usize bitIndex) const noexcept
        {
            MOON_CHECK(bitIndex < BitCount, "bitIndex >= BitCount");
            return Get(GetWord(bitIndex), static_cast<u32>(bitIndex % BitsPerWord));
        }

        /**
         * @brief Checks if all bits are set to true.
         * @return True if every bit is 1.
         */
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

        /**
         * @brief Checks if at least one bit is set to true.
         * @return True if any bit is 1.
         */
        constexpr bool Any() const noexcept
        {
            for (usize currentWord = 0; currentWord < WordCount; ++currentWord)
                if (m_Words[currentWord] != 0)
                    return true;
            return false;
        }

        /**
         * @brief Checks if no bits are set to true.
         * @return True if every bit is 0.
         */
        constexpr bool None() const noexcept
        {
            for (usize currentWord = 0; currentWord < WordCount; ++currentWord)
                if (m_Words[currentWord] != 0)
                    return false;
            return true;
        }

        /**
         * @brief Counts number of bits set to true (popcount).
         * @return Number of 1-bits.
         */
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

        /**
         * @brief Sets all bits to true.
         * @return Reference to this bitset.
         */
        constexpr Bitset& Set() noexcept
        {
            if constexpr (WordCount == 0) return *this;
            for (usize i = 0; i < WordCount - 1; ++i) m_Words[i] = NumericLimit<uptr>::Max;
            m_Words[WordCount - 1] |= NumericLimit<uptr>::Max>>(WordCount * BitsPerWord - BitCount);
            return *this;
        }

        /**
         * @brief Sets or clears bit at @p bitIndex.
         * @param bitIndex 0-based bit index.
         * @param state New boolean state (default true).
         * @return Reference to this bitset.
         */
        constexpr Bitset& Set(usize bitIndex, bool state = true) noexcept
        {
            MOON_CHECK(bitIndex < BitCount, "bitIndex >= BitCount");
            Set(GetWord(bitIndex), bitIndex % BitsPerWord, state);
            return *this;
        }

        /**
         * @brief Clears all bits to false (0).
         * @return Reference to this bitset.
         */
        constexpr Bitset& Reset() noexcept
        {
            for (usize i = 0; i < WordCount; ++i) m_Words[i] = 0;
            return *this;
        }

        /**
         * @brief Clears bit at @p bitIndex to false.
         * @param bitIndex 0-based bit index.
         * @return Reference to this bitset.
         */
        constexpr Bitset& Reset(usize bitIndex) noexcept
        {
            MOON_CHECK(bitIndex < BitCount, "bitIndex >= BitCount");
            Set(GetWord(bitIndex), bitIndex % BitsPerWord, false);
            return *this;
        }

        /**
         * @brief Toggles (inverts) all bits.
         * @return Reference to this bitset.
         */
        constexpr Bitset& Flip() noexcept
        {
            if constexpr (WordCount == 0) return *this;
            for (usize i = 0; i < WordCount - 1; ++i) m_Words[i] = ~m_Words[i];
            m_Words[WordCount - 1] = ~m_Words[WordCount - 1] & NumericLimit<uptr>::Max >> WordCount * BitsPerWord - BitCount;
            return *this;
        }

        /**
         * @brief Toggles (inverts) bit at @p bitIndex.
         * @param bitIndex 0-based bit index.
         * @return Reference to this bitset.
         */
        constexpr Bitset& Flip(usize bitIndex) noexcept
        {
            MOON_CHECK(bitIndex < BitCount, "bitIndex >= BitCount");
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
        uptr m_Words[WordCount == 0 ? 1 : WordCount]{}; ///< Storage words array.
    };

    template <usize BitCount>
    constexpr Bitset<BitCount> Bitset<BitCount>::operator~() const noexcept
    {
        Bitset copy = *this;
        copy.Flip();
        return copy;
    }
}

/** @brief Bitwise binary AND between two bitsets. */
template<usize BitCount>
MC::Bitset<BitCount> operator&(const MC::Bitset<BitCount>& left,
                                         const MC::Bitset<BitCount>& right) noexcept
{
    MC::Bitset<BitCount> result = left;
    return result &= right;
}

/** @brief Bitwise binary OR between two bitsets. */
template<usize BitCount>
MC::Bitset<BitCount> operator|(const MC::Bitset<BitCount>& left,
                                         const MC::Bitset<BitCount>& right) noexcept
{
    MC::Bitset<BitCount> result = left;
    return result |= right;
}

/** @brief Bitwise binary XOR between two bitsets. */
template<usize BitCount>
MC::Bitset<BitCount> operator^(const MC::Bitset<BitCount>& left,
                                         const MC::Bitset<BitCount>& right) noexcept
{
    MC::Bitset<BitCount> result = left;
    return result ^= right;
}
