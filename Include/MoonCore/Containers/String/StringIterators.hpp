#pragma once
#include "MoonCore/Utility/Debug.hpp"
#include "MoonCore/Utility/Iterator.hpp"

/**
 * @file StringIterators.hpp
 * @brief String character iterators and Unicode UTF codepoint decoding iterators.
 * @ingroup string_containers
 */

namespace MC
{
	/**
	 * @class StringIterator
	 * @brief Random-access iterator over raw characters in a string buffer.
	 *
	 * Operates on code units (`char8` or `wchar`).
	 *
	 * @tparam CharacterType Character type (`char8` or `wchar`).
	 * @tparam IsConst Whether this iterator provides const access.
	 */
	template<Concept::Character CharacterType, bool IsConst = false>
	class StringIterator
	{
	public:
		using ValueType	= CharacterType;                               ///< Character code unit type.
		using DiffType	= iptr;                                        ///< Difference type.
		using PtrType	= Conditional<IsConst, const ValueType*, ValueType*>; ///< Pointer to character.
		using RefType	= Conditional<IsConst, const ValueType&, ValueType&>; ///< Reference to character.
		using Category	= Iterator::Tags::RandomAccess;                ///< Random access tag.

	public:
		/** @brief Default constructor. Constructs a null iterator. */
		constexpr StringIterator() noexcept : m_Ptr(nullptr) {}
		/** @brief Constructs iterator pointing to character pointer @p ptr. */
		constexpr StringIterator(PtrType ptr) noexcept : m_Ptr(ptr) {}

	public:
		/** @brief Advances iterator by @p index positions. */
		constexpr StringIterator& operator+=(usize index) { m_Ptr += index; return *this; }
		/** @brief Decrements iterator by @p index positions. */
		constexpr StringIterator& operator-=(usize index) { m_Ptr -= index; return *this; }

		/** @brief Returns iterator advanced by @p index. */
		constexpr StringIterator operator+(usize index) const { return StringIterator(m_Ptr + index); }
		/** @brief Returns iterator moved back by @p index. */
		constexpr StringIterator operator-(usize index) const { return StringIterator(m_Ptr - index); }

		/** @brief Pre-increment operator. */
		constexpr StringIterator& operator++() { ++m_Ptr; return *this; }
		/** @brief Post-increment operator. */
		constexpr StringIterator operator++(int) { StringIterator temp = *this; ++m_Ptr; return temp; }

		/** @brief Pre-decrement operator. */
		constexpr StringIterator& operator--() { --m_Ptr; return (*this); }
		/** @brief Post-decrement operator. */
		constexpr StringIterator operator--(int) { StringIterator temp = *this; --m_Ptr; return temp; }

		/** @brief Computes distance between iterators. */
		constexpr DiffType operator-(const StringIterator& it) const noexcept { return m_Ptr - it.m_Ptr; }

		/** @brief Equality comparison. */
		constexpr bool operator==(const StringIterator& it) const noexcept { return m_Ptr == it.m_Ptr; }
		/** @brief Inequality comparison. */
		constexpr bool operator!=(const StringIterator& it) const noexcept { return m_Ptr != it.m_Ptr; }
		/** @brief Ordering comparison. */
		constexpr bool operator<(const StringIterator& it) const noexcept { return m_Ptr < it.m_Ptr; }
		/** @brief Ordering comparison. */
		constexpr bool operator>(const StringIterator& it) const noexcept { return m_Ptr > it.m_Ptr; }

		/** @brief Subscript access. */
		constexpr RefType operator[](usize index) const { return *(m_Ptr + index); }
		/** @brief Member access operator. */
		constexpr PtrType operator->() const { return m_Ptr; }
		/** @brief Dereference operator. */
		constexpr RefType operator*() const { return *m_Ptr; }
		/** @brief Returns raw underlying character pointer. */
		constexpr PtrType Ptr() const { return m_Ptr; }

	private:
		PtrType m_Ptr; ///< Underlying raw character pointer.
	};

	/**
	 * @class CodepointIterator
	 * @brief Primary template for decoding Unicode codepoints from variable-length character encodings.
	 * @tparam CharacterType Character type.
	 * @tparam IsConst Whether iterator provides const access.
	 */
	template <Concept::Character CharacterType, bool IsConst = false>
	class CodepointIterator;

	/**
	 * @brief Specialization of `CodepointIterator` for UTF-8 character sequences.
	 *
	 * Decodes 1 to 4 UTF-8 bytes on the fly, producing 32-bit Unicode codepoints (`char32`).
	 */
	template <bool IsConst>
	class CodepointIterator<char8, IsConst>
	{
	public:
		using ValueType = char32;                                      ///< Decoded Unicode codepoint type.
		using DiffType = iptr;                                         ///< Distance difference type.
		using PtrType = Conditional<IsConst, const char8*, char8*>;    ///< Pointer to UTF-8 code unit.
		using RefType = Conditional<IsConst, const char32, char32>;    ///< Decoded codepoint value.
		using Category = Iterator::Tags::Bidirectional;                ///< Bidirectional iterator tag.

	public:
		/** @brief Default constructor. Constructs a null iterator. */
		constexpr CodepointIterator() noexcept : m_Ptr(nullptr) {}
		/** @brief Constructs iterator pointing to start of a UTF-8 character. */
		constexpr CodepointIterator(PtrType ptr) noexcept : m_Ptr(ptr) {}

	public:
		/** @brief Advances iterator by @p count codepoints. */
		constexpr CodepointIterator& operator+=(usize count)
		{
			while (count--) ++(*this);
			return *this;
		}
		/** @brief Decrements iterator by @p count codepoints. */
		constexpr CodepointIterator& operator-=(usize count)
		{
			while (count--) --(*this);
			return *this;
		}

		/** @brief Returns iterator advanced by @p count codepoints. */
		constexpr CodepointIterator operator+(usize count) const { CodepointIterator tmp = *this; tmp += count; return tmp; }
		/** @brief Returns iterator decremented by @p count codepoints. */
		constexpr CodepointIterator operator-(usize count) const { CodepointIterator tmp = *this; tmp -= count; return tmp; }

		/**
		 * @brief Advances iterator to next UTF-8 codepoint.
		 * Checks leading byte length and skips continuation bytes.
		 */
		constexpr CodepointIterator& operator++()
		{
			u8 lead = static_cast<u8>(*m_Ptr);
			MOON_CHECK((lead & 0xC0) != 0x80, "Iterator points to a UTF-8 continuation byte");
			if (lead < 0x80) m_Ptr += 1;
			else if (lead < 0xE0) m_Ptr += 2;
			else if (lead < 0xF0) m_Ptr += 3;
			else m_Ptr += 4;
			return *this;
		}

		/**
		 * @brief Decrements iterator to previous UTF-8 codepoint.
		 * Rewinds across any UTF-8 continuation bytes.
		 */
		constexpr CodepointIterator& operator--()
		{
			do --m_Ptr; while ((static_cast<u8>(*m_Ptr) & 0xC0) == 0x80);
			return *this;
		}

		/** @brief Post-increment operator. */
		constexpr CodepointIterator operator++(int) { CodepointIterator tmp = *this; ++(*this); return tmp; }
		/** @brief Post-decrement operator. */
		constexpr CodepointIterator operator--(int) { CodepointIterator tmp = *this; --(*this); return tmp; }

		/** @brief Computes codepoint count between two iterators. */
		constexpr DiffType operator-(const CodepointIterator & other) const
		{
			DiffType count = 0;
			CodepointIterator tmp = (m_Ptr > other.m_Ptr) ? other : *this;
			CodepointIterator end = (m_Ptr > other.m_Ptr) ? *this : other;
			while (tmp != end) { ++tmp; ++count; }
			return (m_Ptr > other.m_Ptr) ? count : -count;
		}

		/** @brief Equality comparison. */
		constexpr bool operator==(const CodepointIterator & it) const noexcept { return m_Ptr == it.m_Ptr; }
		/** @brief Inequality comparison. */
		constexpr bool operator!=(const CodepointIterator & it) const noexcept { return m_Ptr != it.m_Ptr; }
		/** @brief Ordering comparison. */
		constexpr bool operator<(const CodepointIterator & it) const noexcept { return m_Ptr < it.m_Ptr; }
		/** @brief Ordering comparison. */
		constexpr bool operator>(const CodepointIterator & it) const noexcept { return m_Ptr > it.m_Ptr; }

		/**
		 * @brief Dereferences current UTF-8 sequence and decodes into a 32-bit Unicode codepoint.
		 * @return Decoded `char32` value.
		 */
		constexpr RefType operator*() const { return Decode(); }
		/** @brief Access to current code unit. */
		constexpr PtrType operator->() const { return m_Ptr; }
		/** @brief Subscript access. */
		constexpr RefType operator[](usize index) const { return *(*this + index); }
		/** @brief Returns raw code unit pointer. */
		constexpr const char8* Ptr() const { return m_Ptr; }

	private:
		constexpr char32 Decode() const
		{
			u8 lead = static_cast<u8>(*m_Ptr);
			if (lead < 0x80) return lead;
			if (lead < 0xE0) return (lead & 0x1F) << 6 | (m_Ptr[1] & 0x3F);
			if (lead < 0xF0) return (lead & 0x0F) << 12 | (m_Ptr[1] & 0x3F) << 6 | (m_Ptr[2] & 0x3F);
			return (lead & 0x07) << 18 | (m_Ptr[1] & 0x3F) << 12 | (m_Ptr[2] & 0x3F) << 6 | (m_Ptr[3] & 0x3F);
		}

	private:
		PtrType m_Ptr;
	};

	/**
	 * @brief Specialization of `CodepointIterator` for UTF-16 wide character sequences.
	 *
	 * Decodes UTF-16 surrogate pairs into 32-bit Unicode codepoints (`char32`).
	 */
	template <bool IsConst>
	class CodepointIterator<wchar, IsConst>
	{
	public:
		using ValueType = char32;                                      ///< Decoded Unicode codepoint type.
		using DiffType = iptr;                                         ///< Distance difference type.
		using PtrType = Conditional<IsConst, const wchar*, wchar*>;    ///< Pointer to wide character.
		using RefType = Conditional<IsConst, const char32, char32>;    ///< Decoded codepoint value.
		using Category = Iterator::Tags::Bidirectional;                ///< Bidirectional iterator tag.

	public:
		/** @brief Default constructor. Constructs a null iterator. */
		constexpr CodepointIterator() noexcept : m_Ptr(nullptr) {}
		/** @brief Constructs iterator pointing to wide character pointer @p ptr. */
		constexpr CodepointIterator(PtrType ptr) noexcept : m_Ptr(ptr) {}

	public:
		/** @brief Advances iterator by @p count codepoints. */
		constexpr CodepointIterator& operator+=(usize count)
		{
			while (count--) ++(*this);
			return *this;
		}
		/** @brief Decrements iterator by @p count codepoints. */
		constexpr CodepointIterator& operator-=(usize count)
		{
			while (count--) --(*this);
			return *this;
		}

		/** @brief Returns iterator advanced by @p count codepoints. */
		constexpr CodepointIterator operator+(usize count) const { CodepointIterator tmp = *this; tmp += count; return tmp; }
		/** @brief Returns iterator decremented by @p count codepoints. */
		constexpr CodepointIterator operator-(usize count) const { CodepointIterator tmp = *this; tmp -= count; return tmp; }

		/**
		 * @brief Advances iterator to next UTF-16 codepoint, skipping high/low surrogate pairs.
		 */
		constexpr CodepointIterator& operator++()
		{
#ifdef PLATFORM_WINDOWS
			u16 unit = static_cast<u16>(*m_Ptr);
			if ((unit & 0xFC00) == 0xD800) m_Ptr += 2;
			else m_Ptr += 1;
#else
			m_Ptr++;
#endif
		    return *this;
		}

		/**
		 * @brief Decrements iterator to previous UTF-16 codepoint.
		 */
		constexpr CodepointIterator& operator--()
		{
#ifdef PLATFORM_WINDOWS
			do --m_Ptr; while ((static_cast<u16>(*m_Ptr) & 0xFC00) == 0xDC00);
#else
			--m_Ptr;
#endif
		    return *this;
		}

		/** @brief Post-increment operator. */
		constexpr CodepointIterator operator++(int) { CodepointIterator tmp = *this; ++(*this); return tmp; }
		/** @brief Post-decrement operator. */
		constexpr CodepointIterator operator--(int) { CodepointIterator tmp = *this; --(*this); return tmp; }

		/** @brief Computes codepoint count between two iterators. */
		constexpr DiffType operator-(const CodepointIterator& other) const
		{
			DiffType count = 0;
			CodepointIterator tmp = (m_Ptr > other.m_Ptr) ? other : *this;
			CodepointIterator end = (m_Ptr > other.m_Ptr) ? *this : other;
			while (tmp != end) { ++tmp; ++count; }
			return (m_Ptr > other.m_Ptr) ? count : -count;
		}

		/** @brief Equality comparison. */
		constexpr bool operator==(const CodepointIterator& it) const noexcept { return m_Ptr == it.m_Ptr; }
		/** @brief Inequality comparison. */
		constexpr bool operator!=(const CodepointIterator& it) const noexcept { return m_Ptr != it.m_Ptr; }
		/** @brief Ordering comparison. */
		constexpr bool operator<(const CodepointIterator& it) const noexcept { return m_Ptr < it.m_Ptr; }
		/** @brief Ordering comparison. */
		constexpr bool operator>(const CodepointIterator& it) const noexcept { return m_Ptr > it.m_Ptr; }

		/**
		 * @brief Dereferences current UTF-16 code unit and decodes into a 32-bit Unicode codepoint.
		 * @return Decoded `char32` value.
		 */
		constexpr RefType operator*() const { return Decode(); }
		/** @brief Access to current code unit. */
		constexpr PtrType operator->() const { return m_Ptr; }
		/** @brief Subscript access. */
		constexpr RefType operator[](usize index) const { return *(*this + index); }
		/** @brief Returns raw code unit pointer. */
		constexpr const wchar* Ptr() const { return m_Ptr; }

	private:
		constexpr char32 Decode() const
		{
#ifdef PLATFORM_WINDOWS
			u16 unit = static_cast<u16>(*m_Ptr);
			if ((unit & 0xFC00) != 0xD800) return static_cast<char32>(unit);
			u16 high = unit;
			u16 low = static_cast<u16>(*(m_Ptr + 1));
			return 0x10000 + ((static_cast<char32>(high & 0x3FF) << 10) | (low & 0x3FF));
#else
			return static_cast<char32>(*m_Ptr);
#endif
		}

	private:
		PtrType m_Ptr;
	};
}
