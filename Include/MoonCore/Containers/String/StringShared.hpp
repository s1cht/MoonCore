#pragma once
#include "MoonCore/Types.hpp"
#include "MoonCore/Memory/Memory.hpp"
#include "MoonCore/Memory/Allocators/Heap/HeapAllocator.hpp"
#include "StringIterators.hpp"
#include "MoonCore/TypeTraits.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include <cstring>

namespace MC::Concept
{
	template<typename StrType>
	concept IsStringValid = requires
	{
		typename StrType::ValueType;
	} && (sizeof(typename StrType::ValueType) <= 2);

	template<typename StrType>
	concept StringType = IsStringValid<StrType> || Concept::Character<RemoveConst<StrType>>;
}

namespace MC
{
	template<Concept::Character CharacterType>
	using DefaultStringAllocator = Memory::HeapAllocator<CharacterType>;

	template<Concept::Character CharacterType, Concept::HeapAllocatorType Allocator = DefaultStringAllocator<CharacterType>>
	class StringImpl;

	template<Concept::Character CharacterType>
	class StringViewImpl;

	template<Concept::Character CharacterType, bool IsConst = false>
	class CodepointRange
	{
	public:
		using ValueType = Conditional<IsConst, const CharacterType, CharacterType>;

		using Iterator = CodepointIterator<CharacterType, IsConst>;
		using ConstIterator = CodepointIterator<CharacterType, true>;
		
	    using ReverseIterator = MC::Iterator::ReverseIterator<Iterator>;
		using ConstReverseIterator = MC::Iterator::ReverseIterator<ConstIterator>;

	public:
		constexpr CodepointRange(ValueType* begin, ValueType* end) noexcept
			: m_Begin(begin), m_End(end)
	    {}

		constexpr Iterator Begin() const { return Iterator(m_Begin); }
		constexpr ConstIterator CBegin() const { return ConstIterator(m_Begin); }
	    constexpr Iterator End() const { return Iterator(m_End); }
	    constexpr ConstIterator CEnd() const { return ConstIterator(m_End); }

		constexpr ReverseIterator RBegin() const { return ReverseIterator(m_End); }
		constexpr ConstReverseIterator CRBegin() const { return ConstReverseIterator(m_End); }
	    constexpr ReverseIterator REnd() const { return ReverseIterator(m_Begin); }
	    constexpr ConstReverseIterator CREnd() const { return ConstReverseIterator(m_Begin); }

	public:
		constexpr Iterator begin() const { return Begin(); }
		constexpr ConstIterator cbegin() const { return CBegin(); }

		constexpr Iterator end() const { return End(); }
		constexpr ConstIterator cend() const { return CEnd(); }

		constexpr ReverseIterator rbegin() const { return RBegin(); }
		constexpr ConstReverseIterator crbegin() const { return CRBegin(); }

		constexpr ReverseIterator rend() const { return REnd(); }
		constexpr ConstReverseIterator crend() const { return CREnd(); }

	private:
		ValueType* m_Begin;
		ValueType* m_End;
	};

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

	template <Concept::StringType StringType>
    class Convert {};

	template<>
	class Convert<const char8>
	{
	public:
		static i8 ToI8(const char8* string, usize* position = nullptr, i32 base = 10) { return static_cast<i8>(ToI64(string, position, base)); }
		static i16 ToI16(const char8* string, usize* position = nullptr, i32 base = 10) { return static_cast<i16>(ToI64(string, position, base)); }
		static i32 ToI32(const char8* string, usize* position = nullptr, i32 base = 10) { return static_cast<i32>(ToI64(string, position, base)); }
		static i64 ToI64(const char8* string, usize* position = nullptr, i32 base = 10)
		{
			char7* endPtr;
			const char7* castedStr = AS_CHAR7(string);
			u64 result = strtoll(castedStr, &endPtr, base);
			if (position) *position = static_cast<usize>(endPtr - castedStr);
			return result;
		}

		static u8 ToU8(const char8* string, usize* position = nullptr, i32 base = 10) { return static_cast<u8>(ToU64(string, position, base)); }
		static u16 ToU16(const char8* string, usize* position = nullptr, i32 base = 10) { return static_cast<u16>(ToU64(string, position, base)); }
		static u32 ToU32(const char8* string, usize* position = nullptr, i32 base = 10) { return static_cast<u32>(ToU64(string, position, base)); }
		static u64 ToU64(const char8* string, usize* position = nullptr, i32 base = 10)
		{
			char7* endPtr;
			const char7* castedStr = AS_CHAR7(string);
			u64 result = strtoull(castedStr, &endPtr, base);
			if (position) *position = static_cast<usize>(endPtr - castedStr);
			return result;
		}

		static f32 ToF32(const char8* string, usize* position = nullptr)
		{
			char7* endPtr;
			const char7* castedStr = AS_CHAR7(string);
			f32 result = strtof(castedStr, &endPtr);
			if (position) *position = static_cast<usize>(endPtr - castedStr);
			return result;
		}

		static f64 ToF64(const char8* string, usize* position = nullptr)
		{
			char7* endPtr;
			const char7* castedStr = AS_CHAR7(string);
			f64 result = strtod(castedStr, &endPtr);
			if (position) *position = static_cast<usize>(endPtr - castedStr);
			return result;
		}
	};

	template<>
	class Convert<const wchar>
	{
	public:
		static i8 ToI8(const wchar* string, usize* position = nullptr, i32 base = 10) { return static_cast<i8>(ToI64(string, position, base)); }
		static i16 ToI16(const wchar* string, usize* position = nullptr, i32 base = 10) { return static_cast<i16>(ToI64(string, position, base)); }
		static i32 ToI32(const wchar* string, usize* position = nullptr, i32 base = 10) { return static_cast<i32>(ToI64(string, position, base)); }
		static i64 ToI64(const wchar* string, usize* position = nullptr, i32 base = 10)
		{
			wchar* endPtr;
			i64 result = wcstoll(string, &endPtr, base);
			if (position) *position = static_cast<usize>(endPtr - string);
			return result;
		}

		static u8 ToU8(const wchar* string, usize* position = nullptr, i32 base = 10) { return static_cast<u8>(ToU64(string, position, base)); }
		static u16 ToU16(const wchar* string, usize* position = nullptr, i32 base = 10) { return static_cast<u16>(ToU64(string, position, base)); }
		static u32 ToU32(const wchar* string, usize* position = nullptr, i32 base = 10) { return static_cast<u32>(ToU64(string, position, base)); }
		static u64 ToU64(const wchar* string, usize* position = nullptr, i32 base = 10)
		{
			wchar* endPtr;
			u64 result = wcstoull(string, &endPtr, base);
			if (position) *position = static_cast<usize>(endPtr - string);
			return result;
		}

		static f32 ToF32(const wchar* string, usize* position = nullptr)
		{
			wchar* endPtr;
			f32 result = wcstof(string, &endPtr);
			if (position) *position = static_cast<usize>(endPtr - string);
			return result;
		}

		static f64 ToF64(const wchar* string, usize* position = nullptr)
		{
			wchar* endPtr;
			f64 result = wcstod(string, &endPtr);
			if (position) *position = static_cast<usize>(endPtr - string);
			return result;
		}
	};
}