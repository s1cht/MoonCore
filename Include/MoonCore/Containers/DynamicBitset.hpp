#pragma once
#include "MoonCore/Types.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "MoonCore/Utility/Algorithm.hpp"
#include "MoonCore/Containers/Details/Bit.inl"
#include "MoonCore/Containers/String/StringView.hpp"
#include "MoonCore/Memory/Allocators/AllocatorShared.hpp"
#include "MoonCore/Memory/Allocators/Heap/HeapAllocator.hpp"

/**
 * @file DynamicBitset.hpp
 * @brief Dynamic packed boolean bitset container.
 * @ingroup bit_containers
 */

namespace MC
{
    /**
     * @class DynamicBitset
     * @brief Compact runtime-sized sequence of bits packed into machine words.
     *
     * `DynamicBitset<Allocator>` is the heap-allocated counterpart of `Bitset<BitCount>`: the number
     * of bits is chosen at runtime and storage (machine word `uptr` chunks) is obtained from
     * @p Allocator. It provides full bitwise arithmetic operators (`&`, `|`, `^`, `~`, `<<`, `>>`),
     * individual bit query and mutation via reference proxy (`DynamicBitset::Reference`), and set
     * inspection operations (`All()`, `Any()`, `None()`, `Count()`).
     *
     * Bits past `Size()` inside the allocated words are always kept at zero.
     *
     * `Set(bitIndex)` and `Reset(bitIndex)` grow the bitset when @p bitIndex is past the end;
     * `operator[]`, `Test()` and `Flip(bitIndex)` require @p bitIndex to be in range.
     *
     * @tparam Allocator Heap allocator used for word storage. Its `ValueType` must be `uptr`.
     *
     * @code
     * MC::DynamicBitset<> mask(16);
     * mask.Set(2);
     * mask[3] = true;
     * mask.Set(100);                    // grows to 101 bits
     * bool isSet = mask.Test(2);
     * usize totalOnBits = mask.Count();
     * @endcode
     */
    template<Concept::HeapAllocatorType Allocator = Memory::HeapAllocator<uptr>>
        requires Concept::SameAs<typename Allocator::ValueType, uptr>
    class DynamicBitset
    {
    public:
        /**
         * @struct Reference
         * @brief Proxy object representing a single modifiable bit inside a `DynamicBitset`.
         *
         * @warning Invalidated when the bitset reallocates (growth through `Set()`/`Reset()`,
         * `Reserve()`, `Shrink()`).
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
             * @brief Assigns the value of another referenced bit (`a[i] = b[j]`).
             * @param other Proxy to read the value from.
             * @return Reference to this proxy.
             */
            Reference& operator=(const Reference& other) noexcept
            {
                return *this = static_cast<bool>(other);
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

        using ValueType = uptr;           ///< Storage word type.
        using AllocatorType = Allocator;  ///< Allocator used for word storage.

    public:
        static constexpr usize BitsPerWord = sizeof(uptr) * 8; ///< Bits in single machine word.

        /**
         * @brief Returns number of words required to store @p bitCount bits.
         * @param bitCount Number of bits.
         * @return Word count, rounded up.
         */
        static constexpr usize CalculateWordCount(usize bitCount) { return (bitCount + BitsPerWord - 1) / BitsPerWord; }

        /**
         * @brief Returns mask of valid bits in the last word of a @p bitCount-bit sequence.
         * @param bitCount Number of bits.
         * @return Low `bitCount % BitsPerWord` bits set, or all bits if it divides evenly.
         */
        static constexpr uptr TailMask(usize bitCount) noexcept
        {
            const usize remainder = bitCount % BitsPerWord;
            return remainder == 0 ? NumericLimit<uptr>::Max : NumericLimit<uptr>::Max >> (BitsPerWord - remainder);
        }

    public:
        /**
         * @brief Constructs a bitset of @p initialBitCount bits, all zero.
         * @param initialBitCount Initial number of bits (default 64). Zero allocates nothing.
         */
        explicit DynamicBitset(usize initialBitCount = 64) noexcept
        {
            if (initialBitCount == 0) return;
            Allocate(CalculateWordCount(initialBitCount));
            m_BitCount = initialBitCount;
        }

        /**
         * @brief Constructs a bitset from a string of character digits.
         *
         * The last character becomes bit 0, matching `Bitset` (`"100"` sets bit 2).
         * `Size()` equals the string length.
         *
         * @param str String of @p zero and @p one characters.
         * @param zero Character representing 0 (default `'0'`).
         * @param one Character representing 1 (default `'1'`).
         */
        DynamicBitset(const StringView& str,
            char8 zero = TEXT('0'), char8 one = TEXT('1')) noexcept
        {
            Reserve(str.Size());
            m_BitCount = str.Size();

            for (usize i = 0; i < m_BitCount; ++i)
            {
                MOON_CHECK(str[i] == zero || str[i] == one, "Character != zero nor one");
                if (str[m_BitCount - 1 - i] == zero) continue;
                Set(i, true);
            }
        }

        /**
         * @brief Copy constructor. Copies the allocator and all bits.
         * @param other Bitset to copy.
         */
        DynamicBitset(const DynamicBitset& other) noexcept
        {
            m_Allocator = other.m_Allocator;
            m_Words = nullptr;

            if (other.m_WordCount == 0) return;

            Allocate(other.m_WordCount);

            m_BitCount = other.m_BitCount;
            memcpy(m_Words, other.m_Words, m_WordCount * sizeof(uptr));
        }

        /**
         * @brief Move constructor. Takes ownership of @p other's storage; @p other is left empty.
         * @param other Bitset to move from.
         */
        DynamicBitset(DynamicBitset&& other) noexcept
        {
            m_Allocator = Move(other.m_Allocator);
            m_BitCount = other.m_BitCount;
            m_WordCount = other.m_WordCount;
            m_Words = other.m_Words;

            other.m_Words = nullptr;   // the moved-from bitset is empty, not a size without storage
            other.m_WordCount = 0;
            other.m_BitCount = 0;
        }

        /** @brief Destructor. Releases word storage. */
        ~DynamicBitset() noexcept
        {
            if (m_Words)
            {
                m_Allocator.Deallocate(m_Words, m_WordCount);
                m_Words = nullptr;
            }
        }

    public:
        /** @brief Returns current number of bits. */
        constexpr usize Size() const noexcept { return m_BitCount; }

        /** @brief Returns number of bits that fit in the allocated storage without reallocating. */
        constexpr usize Capacity() const noexcept { return m_WordCount * BitsPerWord; }

        /** @brief Returns pointer to the underlying word storage (may be null when empty). */
        uptr* Words() noexcept { return m_Words; }

        /** @brief Returns pointer to the underlying word storage (const overload). */
        const uptr* Words() const noexcept { return m_Words; }

    public:
        /**
         * @brief Ensures storage for at least @p bitCount bits. Does not change `Size()`.
         * @param bitCount Number of bits to reserve.
         * @return False if @p bitCount needs fewer words than currently allocated (nothing is
         *         released; use `Shrink()`), true otherwise.
         */
        bool Reserve(usize bitCount) noexcept
        {
            usize wordCount = CalculateWordCount(bitCount);
            if (wordCount < m_WordCount) return false;
            if (wordCount == m_WordCount) return true;   // nothing to do; don't reallocate
            Allocate(wordCount);
            return true;
        }

        /** @brief Releases storage not needed to hold `Size()` bits. */
        void Shrink() noexcept
        {
            usize wordCount = CalculateWordCount(m_BitCount);
            if (wordCount < m_WordCount) Allocate(wordCount);
        }

    public:
        /**
         * @brief Copy assignment operator. Copies the allocator and all bits.
         * @param other Bitset to copy.
         * @return Reference to this bitset.
         */
        DynamicBitset& operator=(const DynamicBitset& other) noexcept
        {
            if (this == &other) return *this;
            if (m_Words) m_Allocator.Deallocate(m_Words, m_WordCount);

            m_Allocator = other.m_Allocator;
            m_Words = nullptr;
            m_WordCount = 0;
            m_BitCount = 0;

            if (other.m_WordCount == 0) return *this;

            Allocate(other.m_WordCount);

            m_BitCount = other.m_BitCount;
            memcpy(m_Words, other.m_Words, m_WordCount * sizeof(uptr));
            return *this;
        }

        /**
         * @brief Move assignment operator. Takes ownership of @p other's storage; @p other is left empty.
         * @param other Bitset to move from.
         * @return Reference to this bitset.
         */
        DynamicBitset& operator=(DynamicBitset&& other) noexcept
        {
            if (this == &other) return *this;
            if (m_Words) m_Allocator.Deallocate(m_Words, m_WordCount);

            m_Allocator = Move(other.m_Allocator);
            m_BitCount = other.m_BitCount;
            m_WordCount = other.m_WordCount;
            m_Words = other.m_Words;

            other.m_Words = nullptr;
            other.m_WordCount = 0;
            other.m_BitCount = 0;
            return *this;
        }

        /**
         * @brief Accesses bit at @p bitIndex returning a modifiable proxy reference.
         * @param bitIndex 0-based bit index. Must be less than `Size()`.
         * @return Modifiable proxy `Reference`.
         */
        constexpr Reference operator[](usize bitIndex) noexcept
        {
            MOON_CHECK(bitIndex < m_BitCount, "bitIndex >= BitCount");
            return Reference{ m_Words[GetWord(bitIndex)], static_cast<u32>(bitIndex % BitsPerWord) };
        }

        /**
         * @brief Accesses bit at @p bitIndex (const overload).
         * @param bitIndex 0-based bit index. Must be less than `Size()`.
         * @return Boolean value of bit.
         */
        constexpr bool operator[](usize bitIndex) const noexcept
        {
            MOON_CHECK(bitIndex < m_BitCount, "bitIndex >= BitCount");
            return Get(GetWord(bitIndex), static_cast<u32>(bitIndex % BitsPerWord));
        }

        /**
         * @brief Equality comparison between two bitsets.
         * @param other Bitset to compare.
         * @return True if sizes and bit patterns are identical. Capacity is ignored.
         */
        constexpr bool operator==(const DynamicBitset& other) const noexcept
        {
            if (other.m_BitCount != m_BitCount) return false;
            const usize usedWords = UsedWords();   // same for both, because the sizes match
            for (usize i = 0; i < usedWords; ++i)
                if (m_Words[i] != other.m_Words[i]) return false;
            return true;
        }

        /** @brief Bitwise AND assignment operator. Sizes must match. */
        constexpr DynamicBitset& operator&=(const DynamicBitset& other) noexcept
        {
            MOON_CHECK(other.m_BitCount == m_BitCount, "Bitwise operation on bitsets of different sizes");
            const usize usedWords = UsedWords();
            for (usize i = 0; i < usedWords; i++)
                m_Words[i] &= other.m_Words[i];
            return *this;
        }

        /** @brief Bitwise OR assignment operator. Sizes must match. */
        constexpr DynamicBitset& operator|=(const DynamicBitset& other) noexcept
        {
            MOON_CHECK(other.m_BitCount == m_BitCount, "Bitwise operation on bitsets of different sizes");
            const usize usedWords = UsedWords();
            for (usize i = 0; i < usedWords; i++)
                m_Words[i] |= other.m_Words[i];
            return *this;
        }

        /** @brief Bitwise XOR assignment operator. Sizes must match. */
        constexpr DynamicBitset& operator^=(const DynamicBitset& other) noexcept
        {
            MOON_CHECK(other.m_BitCount == m_BitCount, "Bitwise operation on bitsets of different sizes");
            const usize usedWords = UsedWords();
            for (usize i = 0; i < usedWords; i++)
                m_Words[i] ^= other.m_Words[i];
            return *this;
        }

        /** @brief Bitwise NOT operator. Returns a copy with all bits inverted. */
        constexpr DynamicBitset operator~() const noexcept
        {
            DynamicBitset copy = *this;
            copy.Flip();
            return copy;
        }

        /**
         * @brief Bitwise right shift assignment operator (towards bit 0).
         *
         * Vacated high bits become 0. Shifting by `Size()` or more clears the bitset.
         */
        constexpr DynamicBitset& operator>>=(usize position) noexcept
        {
            if (position >= m_BitCount) return Reset();
            const usize usedWords = UsedWords();
            const usize wordShift = position / BitsPerWord;
            if (wordShift != 0)
                for (usize i = 0; i < usedWords; ++i)
                    m_Words[i] = i + wordShift < usedWords ? m_Words[i + wordShift] : 0;
            if ((position %= BitsPerWord) != 0)
            {
                for (usize i = 0; i + 1 < usedWords; ++i)
                    m_Words[i] = (m_Words[i] >> position) | (m_Words[i + 1] << (BitsPerWord - position));
                m_Words[usedWords - 1] >>= position;
            }
            return *this;
        }

        /** @brief Bitwise right shift operator. Returns a shifted copy. */
        DynamicBitset operator>>(usize position) const noexcept
        {
            DynamicBitset result = *this;
            result >>= position;
            return result;
        }

        /**
         * @brief Bitwise left shift assignment operator (towards higher bits).
         *
         * Bits shifted past `Size()` are discarded; the bitset does not grow.
         * Shifting by `Size()` or more clears the bitset.
         */
        constexpr DynamicBitset& operator<<=(usize position) noexcept
        {
            if (position >= m_BitCount) return Reset();
            const usize usedWords = UsedWords();
            const usize wordShift = position / BitsPerWord;
            if (wordShift != 0)
                for (usize i = usedWords; i-- > 0;)
                    m_Words[i] = wordShift <= i ? m_Words[i - wordShift] : 0;
            if ((position %= BitsPerWord) != 0)
            {
                for (usize i = usedWords - 1; i > 0; --i)
                    m_Words[i] = (m_Words[i] << position) | (m_Words[i - 1] >> (BitsPerWord - position));
                m_Words[0] <<= position;
            }
            Trim();
            return *this;
        }

        /** @brief Bitwise left shift operator. Returns a shifted copy. */
        DynamicBitset operator<<(usize position) const noexcept
        {
            DynamicBitset result = *this;
            result <<= position;
            return result;
        }

    public:
        /**
         * @brief Tests value of bit at @p bitIndex.
         * @param bitIndex 0-based bit index. Must be less than `Size()`.
         * @return True if set, false if clear.
         */
        constexpr bool Test(usize bitIndex) const noexcept
        {
            MOON_CHECK(bitIndex < m_BitCount, "bitIndex >= BitCount");
            return Get(GetWord(bitIndex), static_cast<u32>(bitIndex % BitsPerWord));
        }

        /**
         * @brief Checks if all bits are set to true.
         * @return True if every bit is 1. True for an empty bitset.
         */
        constexpr bool All() const noexcept
        {
            const usize usedWords = UsedWords();
            if (usedWords == 0) return true;
            for (usize i = 0; i + 1 < usedWords; ++i)
                if (m_Words[i] != NumericLimit<uptr>::Max) return false;
            return m_Words[usedWords - 1] == TailMask(m_BitCount);   // padding is 0, so == is exact
        }

        /**
         * @brief Checks if at least one bit is set to true.
         * @return True if any bit is 1.
         */
        constexpr bool Any() const noexcept
        {
            const usize usedWords = UsedWords();
            for (usize i = 0; i < usedWords; ++i)
                if (m_Words[i] != 0)
                    return true;
            return false;
        }

        /**
         * @brief Checks if no bits are set to true.
         * @return True if every bit is 0.
         */
        constexpr bool None() const noexcept
        {
            return !Any();
        }

        /**
         * @brief Counts number of bits set to true (popcount).
         * @return Number of 1-bits.
         */
        constexpr usize Count() const noexcept
        {
            usize result = 0;
            const usize usedWords = UsedWords();
            for (usize i = 0; i < usedWords; ++i)
            {
                uptr word = m_Words[i];
                for (usize chunk = 0; chunk < BitsPerWord; chunk += 16)
                    result += Details::PopCount(static_cast<u16>(word >> chunk));
            }
            return result;
        }

        /**
         * @brief Sets all bits to true.
         * @return Reference to this bitset.
         */
        constexpr DynamicBitset& Set() noexcept
        {
            const usize usedWords = UsedWords();
            for (usize i = 0; i < usedWords; ++i) m_Words[i] = NumericLimit<uptr>::Max;
            Trim();
            return *this;
        }

        /**
         * @brief Sets or clears bit at @p bitIndex, growing the bitset if needed.
         *
         * If @p bitIndex >= `Size()`, `Size()` becomes `bitIndex + 1` and the new bits are 0.
         *
         * @param bitIndex 0-based bit index.
         * @param state New boolean state (default true).
         * @return Reference to this bitset.
         */
        DynamicBitset& Set(usize bitIndex, bool state = true) noexcept
        {
            Set(GetWord(bitIndex), bitIndex % BitsPerWord, state);
            return *this;
        }

        /**
         * @brief Clears all bits to false (0). Size and capacity are unchanged.
         * @return Reference to this bitset.
         */
        constexpr DynamicBitset& Reset() noexcept
        {
            for (usize i = 0; i < m_WordCount; ++i) m_Words[i] = 0;
            return *this;
        }

        /**
         * @brief Clears bit at @p bitIndex to false, growing the bitset if needed (see `Set(usize, bool)`).
         * @param bitIndex 0-based bit index.
         * @return Reference to this bitset.
         */
        constexpr DynamicBitset& Reset(usize bitIndex) noexcept
        {
            Set(GetWord(bitIndex), bitIndex % BitsPerWord, false);
            return *this;
        }

        /**
         * @brief Toggles (inverts) all bits.
         * @return Reference to this bitset.
         */
        constexpr DynamicBitset& Flip() noexcept
        {
            const usize usedWords = UsedWords();
            for (usize i = 0; i < usedWords; ++i) m_Words[i] = ~m_Words[i];
            Trim();
            return *this;
        }

        /**
         * @brief Toggles (inverts) bit at @p bitIndex.
         * @param bitIndex 0-based bit index. Must be less than `Size()`.
         * @return Reference to this bitset.
         */
        constexpr DynamicBitset& Flip(usize bitIndex) noexcept
        {
            Set(GetWord(bitIndex), bitIndex % BitsPerWord, !Test(bitIndex));
            return *this;
        }

    private:
        void Set(usize word, usize bit, bool state) noexcept
        {
            if (word >= m_WordCount) Allocate(word + 1);
            m_BitCount = Max(word * BitsPerWord + bit + 1, m_BitCount);
            m_Words[word] = (m_Words[word] & ~(static_cast<uptr>(1) << bit)) | (static_cast<uptr>(state) << bit);
        }

        constexpr bool Get(usize word, usize bit) const noexcept
        {
            return (m_Words[word] & (static_cast<uptr>(1) << bit)) != 0;
        }

        static constexpr usize GetWord(usize bitIndex) noexcept
        {
            return bitIndex / BitsPerWord;
        }

        constexpr usize UsedWords() const noexcept
        {
            return CalculateWordCount(m_BitCount);
        }

    private:
        constexpr void Trim() noexcept
        {
            const usize usedWords = UsedWords();
            if (usedWords == 0) return;
            m_Words[usedWords - 1] &= TailMask(m_BitCount);
        }

    private:
        void Allocate(usize newWordCount) noexcept
        {
            ValueType* newBlock = m_Allocator.Allocate(newWordCount);

            usize elementsToMove = (m_Words && m_WordCount > 0) ? Min(m_WordCount, newWordCount) : 0;

            memset(newBlock, 0, newWordCount * sizeof(uptr));

            if (m_Words && m_WordCount > 0)
            {
                memmove(newBlock, m_Words, elementsToMove * sizeof(ValueType));
                m_Allocator.Deallocate(m_Words, m_WordCount);
            }

            m_Words = newBlock;
            m_WordCount = newWordCount;

            m_BitCount = Min(m_BitCount, m_WordCount * BitsPerWord);
        }

    private:
        uptr* m_Words = nullptr;  ///< Storage words array.
        usize m_WordCount = 0;    ///< Number of allocated words.
        usize m_BitCount = 0;     ///< Number of bits in use.

        AllocatorType m_Allocator; ///< Word storage allocator.
    };

    /** @brief Bitwise binary AND between two dynamic bitsets. Sizes must match. */
    template<MC::Concept::HeapAllocatorType Allocator>
    MC::DynamicBitset<Allocator> operator&(const MC::DynamicBitset<Allocator>& left,
        const MC::DynamicBitset<Allocator>& right) noexcept
    {
        MC::DynamicBitset<Allocator> result = left;
        result &= right;
        return result;
    }

    /** @brief Bitwise binary OR between two dynamic bitsets. Sizes must match. */
    template<MC::Concept::HeapAllocatorType Allocator>
    MC::DynamicBitset<Allocator> operator|(const MC::DynamicBitset<Allocator>& left,
        const MC::DynamicBitset<Allocator>& right) noexcept
    {
        MC::DynamicBitset<Allocator> result = left;
        result |= right;
        return result;
    }

    /** @brief Bitwise binary XOR between two dynamic bitsets. Sizes must match. */
    template<MC::Concept::HeapAllocatorType Allocator>
    MC::DynamicBitset<Allocator> operator^(const MC::DynamicBitset<Allocator>& left,
        const MC::DynamicBitset<Allocator>& right) noexcept
    {
        MC::DynamicBitset<Allocator> result = left;
        result ^= right;
        return result;
    }
}