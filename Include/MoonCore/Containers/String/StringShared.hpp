#pragma once
#include "MoonCore/Types.hpp"
#include "MoonCore/Memory/Memory.hpp"
#include "MoonCore/Memory/Allocators/Heap/HeapAllocator.hpp"
#include "MoonCore/Containers/String/StringIterators.hpp"
#include "MoonCore/TypeTraits.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include <cstring>

/**
 * @file StringShared.hpp
 * @brief Common types, concepts, codepoint ranges, and parsing utilities for strings.
 * @ingroup string_containers
 */

namespace MC::Concept
{
	/** @brief Concept checking if a string type has `ValueType` with size <= 2 bytes. */
	template<typename StrType>
	concept IsStringValid = requires
	{
		typename StrType::ValueType;
	} && (sizeof(typename StrType::ValueType) <= 2);

	/** @brief Concept matching valid string containers or character types. */
	template<typename StrType>
	concept StringType = IsStringValid<StrType> || Concept::Character<RemoveConst<StrType>>;
}

namespace MC
{
	/** @brief Default heap allocator policy for string character storage. */
	template<Concept::Character CharacterType>
	using DefaultStringAllocator = Memory::HeapAllocator<CharacterType>;

	template<Concept::Character CharacterType, Concept::HeapAllocatorType Allocator = DefaultStringAllocator<CharacterType>>
	class StringImpl;

	template<Concept::Character CharacterType>
	class StringViewImpl;

	/**
	 * @class CodepointRange
	 * @brief Range object for iterating over Unicode codepoints in a UTF-8 or UTF-16 string buffer.
	 *
	 * Provides standard range iterators (`Begin()`, `End()`) that decode multi-byte UTF sequences.
	 *
	 * @tparam CharacterType Character type (`char8` or `wchar`).
	 * @tparam IsConst Whether the range provides const access.
	 */
	template<Concept::Character CharacterType, bool IsConst = false>
	class CodepointRange
	{
	public:
		using ValueType = Conditional<IsConst, const CharacterType, CharacterType>;   ///< Character value type.

		using Iterator = CodepointIterator<CharacterType, IsConst>;                  ///< Codepoint iterator.
		using ConstIterator = CodepointIterator<CharacterType, true>;                 ///< Const codepoint iterator.
		
	    using ReverseIterator = MC::Iterator::ReverseIterator<Iterator>;             ///< Reverse codepoint iterator.
		using ConstReverseIterator = MC::Iterator::ReverseIterator<ConstIterator>;   ///< Const reverse codepoint iterator.

	public:
		/**
		 * @brief Constructs a codepoint range covering pointers `[begin, end)`.
		 * @param begin Beginning pointer.
		 * @param end Past-the-end pointer.
		 */
		constexpr CodepointRange(ValueType* begin, ValueType* end) noexcept
			: m_Begin(begin), m_End(end)
	    {}

		/** @brief Returns codepoint iterator to beginning. */
		constexpr Iterator Begin() const { return Iterator(m_Begin); }
		/** @brief Returns const codepoint iterator to beginning. */
		constexpr ConstIterator CBegin() const { return ConstIterator(m_Begin); }
		/** @brief Returns codepoint iterator to end. */
	    constexpr Iterator End() const { return Iterator(m_End); }
		/** @brief Returns const codepoint iterator to end. */
	    constexpr ConstIterator CEnd() const { return ConstIterator(m_End); }

		/** @brief Returns reverse codepoint iterator to beginning of reversed range. */
		constexpr ReverseIterator RBegin() const { return ReverseIterator(m_End); }
		/** @brief Returns const reverse codepoint iterator to beginning of reversed range. */
		constexpr ConstReverseIterator CRBegin() const { return ConstReverseIterator(m_End); }
		/** @brief Returns reverse codepoint iterator to end of reversed range. */
	    constexpr ReverseIterator REnd() const { return ReverseIterator(m_Begin); }
		/** @brief Returns const reverse codepoint iterator to end of reversed range. */
	    constexpr ConstReverseIterator CREnd() const { return ConstReverseIterator(m_Begin); }

	public:
		/** @brief Range-for compatible begin iterator. */
		constexpr Iterator begin() const { return Begin(); }
		/** @brief Range-for compatible const begin iterator. */
		constexpr ConstIterator cbegin() const { return CBegin(); }
		/** @brief Range-for compatible end iterator. */
		constexpr Iterator end() const { return End(); }
		/** @brief Range-for compatible const end iterator. */
		constexpr ConstIterator cend() const { return CEnd(); }

		/** @brief Range-for compatible reverse begin iterator. */
		constexpr ReverseIterator rbegin() const { return RBegin(); }
		/** @brief Range-for compatible const reverse begin iterator. */
		constexpr ConstReverseIterator crbegin() const { return CRBegin(); }
		/** @brief Range-for compatible reverse end iterator. */
		constexpr ReverseIterator rend() const { return REnd(); }
		/** @brief Range-for compatible const reverse end iterator. */
		constexpr ConstReverseIterator crend() const { return CREnd(); }

	private:
		ValueType* m_Begin; ///< Pointer to start of character buffer.
		ValueType* m_End;   ///< Pointer to end of character buffer.
	};

	/**
	 * @brief Computes length of null-terminated string at compile time or runtime.
	 * @tparam T Character type.
	 * @param str Null-terminated character pointer.
	 * @return Character count excluding null terminator.
	 */
	template<typename T>
	static constexpr usize GetStringSize(const T* str) noexcept
	{
		usize size = 0;
		while (*str != 0)
		{
			++size;
			++str;
		}
		return size;
	}

	/**
	 * @class Convert
	 * @brief Conversion utility template for converting strings to primitive numeric types.
	 * @tparam StringType String or character type.
	 */
	template <Concept::StringType StringType>
    class Convert {};

	/**
	 * @brief Specialization of `Convert` for UTF-8 null-terminated character buffers.
	 */
	template<>
	class Convert<const char8>
	{
	public:
		/** @brief Parses 8-bit signed integer. */
		static i8 ToI8(const char8* string, usize* position = nullptr, i32 base = 10) { return static_cast<i8>(ToI64(string, position, base)); }
		/** @brief Parses 16-bit signed integer. */
		static i16 ToI16(const char8* string, usize* position = nullptr, i32 base = 10) { return static_cast<i16>(ToI64(string, position, base)); }
		/** @brief Parses 32-bit signed integer. */
		static i32 ToI32(const char8* string, usize* position = nullptr, i32 base = 10) { return static_cast<i32>(ToI64(string, position, base)); }
		/** @brief Parses 64-bit signed integer. */
		static i64 ToI64(const char8* string, usize* position = nullptr, i32 base = 10)
		{
			char7* endPtr;
			const char7* castedStr = AS_CHAR7(string);
			u64 result = strtoll(castedStr, &endPtr, base);
			if (position) *position = static_cast<usize>(endPtr - castedStr);
			return result;
		}

		/** @brief Parses 8-bit unsigned integer. */
		static u8 ToU8(const char8* string, usize* position = nullptr, i32 base = 10) { return static_cast<u8>(ToU64(string, position, base)); }
		/** @brief Parses 16-bit unsigned integer. */
		static u16 ToU16(const char8* string, usize* position = nullptr, i32 base = 10) { return static_cast<u16>(ToU64(string, position, base)); }
		/** @brief Parses 32-bit unsigned integer. */
		static u32 ToU32(const char8* string, usize* position = nullptr, i32 base = 10) { return static_cast<u32>(ToU64(string, position, base)); }
		/** @brief Parses 64-bit unsigned integer. */
		static u64 ToU64(const char8* string, usize* position = nullptr, i32 base = 10)
		{
			char7* endPtr;
			const char7* castedStr = AS_CHAR7(string);
			u64 result = strtoull(castedStr, &endPtr, base);
			if (position) *position = static_cast<usize>(endPtr - castedStr);
			return result;
		}

		/** @brief Parses single-precision 32-bit float. */
		static f32 ToF32(const char8* string, usize* position = nullptr)
		{
			char7* endPtr;
			const char7* castedStr = AS_CHAR7(string);
			f32 result = strtof(castedStr, &endPtr);
			if (position) *position = static_cast<usize>(endPtr - castedStr);
			return result;
		}

		/** @brief Parses double-precision 64-bit float. */
		static f64 ToF64(const char8* string, usize* position = nullptr)
		{
			char7* endPtr;
			const char7* castedStr = AS_CHAR7(string);
			f64 result = strtod(castedStr, &endPtr);
			if (position) *position = static_cast<usize>(endPtr - castedStr);
			return result;
		}
	};

	/**
	 * @brief Specialization of `Convert` for wide string (UTF-16) character buffers.
	 */
	template<>
	class Convert<const wchar>
	{
	public:
		/** @brief Parses 8-bit signed integer. */
		static i8 ToI8(const wchar* string, usize* position = nullptr, i32 base = 10) { return static_cast<i8>(ToI64(string, position, base)); }
		/** @brief Parses 16-bit signed integer. */
		static i16 ToI16(const wchar* string, usize* position = nullptr, i32 base = 10) { return static_cast<i16>(ToI64(string, position, base)); }
		/** @brief Parses 32-bit signed integer. */
		static i32 ToI32(const wchar* string, usize* position = nullptr, i32 base = 10) { return static_cast<i32>(ToI64(string, position, base)); }
		/** @brief Parses 64-bit signed integer. */
		static i64 ToI64(const wchar* string, usize* position = nullptr, i32 base = 10)
		{
			wchar* endPtr;
			i64 result = wcstoll(string, &endPtr, base);
			if (position) *position = static_cast<usize>(endPtr - string);
			return result;
		}

		/** @brief Parses 8-bit unsigned integer. */
		static u8 ToU8(const wchar* string, usize* position = nullptr, i32 base = 10) { return static_cast<u8>(ToU64(string, position, base)); }
		/** @brief Parses 16-bit unsigned integer. */
		static u16 ToU16(const wchar* string, usize* position = nullptr, i32 base = 10) { return static_cast<u16>(ToU64(string, position, base)); }
		/** @brief Parses 32-bit unsigned integer. */
		static u32 ToU32(const wchar* string, usize* position = nullptr, i32 base = 10) { return static_cast<u32>(ToU64(string, position, base)); }
		/** @brief Parses 64-bit unsigned integer. */
		static u64 ToU64(const wchar* string, usize* position = nullptr, i32 base = 10)
		{
			wchar* endPtr;
			u64 result = wcstoull(string, &endPtr, base);
			if (position) *position = static_cast<usize>(endPtr - string);
			return result;
		}

		/** @brief Parses single-precision 32-bit float. */
		static f32 ToF32(const wchar* string, usize* position = nullptr)
		{
			wchar* endPtr;
			f32 result = wcstof(string, &endPtr);
			if (position) *position = static_cast<usize>(endPtr - string);
			return result;
		}

		/** @brief Parses double-precision 64-bit float. */
		static f64 ToF64(const wchar* string, usize* position = nullptr)
		{
			wchar* endPtr;
			f64 result = wcstod(string, &endPtr);
			if (position) *position = static_cast<usize>(endPtr - string);
			return result;
		}
	};
}
