#pragma once
#include "MoonCore/Containers/String/StringShared.hpp"
#include "MoonCore/Containers/Bitset.hpp"

namespace MC
{
	template<Concept::Character CharacterType>
	class StringViewImpl
	{
	public:
		using ValueType		= CharacterType;
		using PtrType		= const ValueType*;
		using ConstPtrType	= const ValueType*;
		using RefType		= const ValueType&;
		using ConstRefType	= const ValueType&;

		using Iterator = StringIterator<CharacterType, true>;
		using ConstIterator = Iterator;
		using ReverseIterator = MC::Iterator::ReverseIterator<Iterator>;
		using ConstReverseIterator = ReverseIterator;

	public:
		static constexpr usize NoPosition = NumericLimit<usize>::Max;

	public:
		constexpr StringViewImpl() noexcept
			: m_Data(nullptr), m_Size(0)
		{}

		constexpr StringViewImpl(const char7* data) noexcept
			: m_Data(AS_UTF8(data)), m_Size(data ? GetStringSize(data) : 0)
		{
			static_assert(sizeof(ValueType) == 1, "This constructor is available for only UTF8 string!");
		}

		constexpr StringViewImpl(const char7* data, usize size) noexcept
			: m_Data(AS_UTF8(data)), m_Size(size)
		{
			static_assert(sizeof(ValueType) == 1, "This constructor is available for only UTF8 string!");
		}

		constexpr StringViewImpl(PtrType data) noexcept
			: m_Data(data), m_Size(data ? GetStringSize(data) : 0)
		{}

		constexpr StringViewImpl(PtrType data, usize size) noexcept
			: m_Data(data), m_Size(size)
		{}

		template<class Allocator>
		StringViewImpl(const StringImpl<ValueType, Allocator>& str) noexcept
			: m_Data(str.String()), m_Size(str.Size())
		{}

		constexpr StringViewImpl(const StringViewImpl& other) noexcept = default;
		constexpr StringViewImpl(StringViewImpl&& other) noexcept = default;

		constexpr ~StringViewImpl() noexcept = default;

	public:
		constexpr usize Size() const noexcept { return m_Size; }
		constexpr PtrType String() const noexcept { return m_Data; }
		constexpr bool Empty() const noexcept { return m_Size <= 0; }

	public:
		constexpr Iterator Begin() const noexcept { return Iterator(m_Data); }
		constexpr Iterator CBegin() const noexcept { return Iterator(m_Data); }
		constexpr Iterator End() const noexcept { return Iterator(m_Data + m_Size); }
		constexpr Iterator CEnd() const noexcept { return Iterator(m_Data + m_Size); }

		constexpr ReverseIterator RBegin() const noexcept { return m_Size > 0 ? ReverseIterator(m_Data + m_Size - 1) : REnd(); }
		constexpr ReverseIterator CRBegin() const noexcept { return m_Size > 0 ? ReverseIterator(m_Data + m_Size - 1) : CREnd(); }
		constexpr ReverseIterator REnd() const noexcept { return ReverseIterator(m_Data - 1); }
		constexpr ReverseIterator CREnd() const noexcept { return ReverseIterator(m_Data - 1); }

	public:
		constexpr Iterator begin() const noexcept { return Begin(); }
		constexpr Iterator cbegin() const noexcept { return CBegin(); }

		constexpr Iterator end() const noexcept { return End(); }
		constexpr Iterator cend() const noexcept { return CEnd(); }

		constexpr ReverseIterator rbegin() const noexcept { return RBegin(); }
		constexpr ReverseIterator crbegin() const noexcept { return CRBegin(); }

		constexpr ReverseIterator rend() const noexcept { return REnd(); }
		constexpr ReverseIterator crend() const noexcept { return CREnd(); }

	public:
		constexpr CodepointRange<CharacterType, true>::Iterator CodepointBegin() const noexcept
		{
			return CodepointRange<CharacterType, true>::Iterator(m_Data);
		}

		constexpr CodepointRange<CharacterType, true>::Iterator CodepointEnd() const noexcept
		{
			return CodepointRange<CharacterType, true>::Iterator(m_Data + m_Size);
		}

		constexpr CodepointRange<CharacterType, true> Codepoints() const noexcept
		{
			return CodepointRange<CharacterType, true>(m_Data, m_Data + m_Size);
		}

		constexpr usize CodepointCount() const noexcept
		{
			usize count = 0;
			for (usize i = 0; i < m_Size; ++i)
				if ((static_cast<u8>(m_Data[i]) & 0xC0) != 0x80)
					++count;
			return count;
		}

		constexpr char32 GetCodepoint(usize codepointIndex) const noexcept
		{
			auto it = CodepointBegin();
			const auto e = CodepointEnd();
			for (usize i = 0; i < codepointIndex; ++i)
			{
				MC_CHECK(it != e, "Codepoint index out of range");
				++it;
			}
			MC_CHECK(it != e, "Codepoint index out of range");
			return *it;
		}

		constexpr usize CodepointOffset(usize codepointIndex) const noexcept
		{
			auto it = CodepointBegin();
			const auto e = CodepointEnd();
			for (usize i = 0; i < codepointIndex; ++i)
			{
				MC_CHECK(it != e, "Codepoint index out of range");
				++it;
			}
			return static_cast<usize>(it.Ptr() - m_Data);
		}

		constexpr StringViewImpl CodepointSubstring(usize startCodepoint, usize codepointCount) const noexcept
		{
			usize startByte = CodepointOffset(startCodepoint);
			auto it = CodepointRange<CharacterType, true>::Iterator(m_Data + startByte);
			const auto e = CodepointEnd();
			for (usize i = 0; i < codepointCount && it != e; ++i)
				++it;
			usize endByte = static_cast<usize>(it.Ptr() - m_Data);
			return PStringView(m_Data + startByte, endByte - startByte);
		}

	public:
		constexpr RefType operator[](const usize index) const noexcept
		{
			MC_CHECK(index < m_Size, "Index in string is out of range");
			return m_Data[index];
		}

		constexpr bool operator==(const ValueType* str) const noexcept
		{
			if (m_Size != GetStringSize(str)) return false;
			for (size_t i = 0; i < m_Size; ++i)
				if (m_Data[i] != str[i])
					return false;
			return true;
		}

		constexpr bool operator==(const StringViewImpl& other) const noexcept
		{
			if (m_Size != other.m_Size) return false;
			for (size_t i = 0; i < m_Size; ++i)
				if (m_Data[i] != other.m_Data[i])
					return false;
			return true;
		}

		constexpr StringViewImpl& operator=(const StringViewImpl&) noexcept = default;
		constexpr StringViewImpl& operator=(StringViewImpl&&) noexcept = default;

		constexpr StringViewImpl Substring(usize start, usize length = NoPosition) const noexcept
		{
			MC_CHECK(start < m_Size, "Start index is out of range");
			usize remaining = m_Size - start;
			usize newSize = (length > remaining) ? remaining : length;
			return StringViewImpl(m_Data + start, newSize);
		}

		template<Concept::HeapAllocatorType Allocator = DefaultStringAllocator<CharacterType>>
		StringImpl<ValueType, Allocator> ToString() const noexcept
		{
			return StringImpl<ValueType, Allocator>(m_Data, m_Size);
		}

	public:
		constexpr usize Find(const StringViewImpl& in, usize pos = 0) const noexcept
		{
			return PFind(in.String(), in.Size(), pos);
		}

		constexpr usize Find(const ValueType* in, usize pos = 0) const noexcept
		{
			return PFind(in, GetStringSize(in), pos);
		}

		constexpr usize Find(const ValueType* in, usize pos, usize n) const noexcept
		{
			return PFind(in, n, pos);
		}

		constexpr usize Find(ValueType in, usize pos = 0) const noexcept
		{
			return PFind(const_cast<const ValueType*>(&in), 1, pos);
		}

		constexpr usize FindFirst(const StringViewImpl& in, usize pos = 0) const noexcept
		{
			return PFindFirst(in.String(), in.Size(), pos);
		}

		constexpr usize FindFirst(const ValueType* in, usize pos = 0) const noexcept
		{
			return PFindFirst(in, GetStringSize(in), pos);
		}

		constexpr usize FindFirst(const ValueType* in, usize pos, usize n) const noexcept
		{
			return PFindFirst(in, n, pos);
		}

		constexpr usize FindFirst(ValueType in, usize pos = 0) const noexcept
		{
			return PFindFirst(const_cast<const ValueType*>(&in), 1, pos);
		}


		constexpr usize FindFirstNot(const StringViewImpl& in, usize pos = 0) const noexcept
		{
			return PFindFirstNot(in.String(), in.Size(), pos);
		}

		constexpr usize FindFirstNot(const ValueType* in, usize pos = 0) const noexcept
		{
			return PFindFirstNot(in, GetStringSize(in), pos);
		}

		constexpr usize FindFirstNot(const ValueType* in, usize pos, usize n) const noexcept
		{
			return PFindFirstNot(in, n, pos);
		}

		constexpr usize FindFirstNot(ValueType in, usize pos = 0) const noexcept
		{
			return PFindFirstNot(const_cast<const ValueType*>(&in), 1, pos);
		}

		constexpr usize FindLast(const StringViewImpl& in, usize pos = NoPosition) const noexcept
		{
			if (pos == NoPosition) pos = m_Size - 1;
			return PFindLast(in.String(), in.Size(), pos);
		}

		constexpr usize FindLast(const ValueType* in, usize pos = NoPosition) const noexcept
		{
			if (pos == NoPosition) pos = m_Size - 1;
			return PFindLast(in, GetStringSize(in), pos);
		}

		constexpr usize FindLast(const ValueType* in, usize pos, usize n) const noexcept
		{
			if (pos == NoPosition) pos = m_Size - 1;
			return PFindLast(in, n, pos);
		}

		constexpr usize FindLast(ValueType in, usize pos = NoPosition) const noexcept
		{
			if (pos == NoPosition) pos = m_Size - 1;
			return PFindLast(const_cast<const ValueType*>(&in), 1, pos);
		}


		constexpr usize FindLastNot(const StringViewImpl& in, usize pos = NoPosition) const noexcept
		{
			if (pos == NoPosition) pos = m_Size - 1;
			return PFindLastNot(in.String(), in.Size(), pos);
		}

		constexpr usize FindLastNot(const ValueType* in, usize pos = NoPosition) const noexcept
		{
			if (pos == NoPosition) pos = m_Size - 1;
			return PFindLastNot(in, GetStringSize(in), pos);
		}

		constexpr usize FindLastNot(const ValueType* in, usize pos, usize n) const noexcept
		{
			if (pos == NoPosition) pos = m_Size - 1;
			return PFindLastNot(in, n, pos);
		}

		constexpr usize FindLastNot(ValueType in, usize pos = NoPosition) const noexcept
		{
			if (pos == NoPosition) pos = m_Size - 1;
			return PFindLastNot(const_cast<const ValueType*>(&in), 1, pos);
		}

	private:
		constexpr usize PFind(const ValueType* needle, usize needleSize, usize startAt) const noexcept
		{
			if (needleSize == 0 || startAt >= m_Size || needleSize > m_Size - startAt) return NoPosition;

			const ValueType firstChar = needle[0];
			usize limit = m_Size - needleSize;
			for (usize i = startAt; i <= limit; ++i)
			{
				if (m_Data[i] == firstChar)
				{
					bool match = true;
					for (usize j = 1; j < needleSize; ++j)
					{
						if (m_Data[i + j] != needle[j])
						{
							match = false;
							break;
						}
					}
					if (match)
						return i;
				}
			}
			return NoPosition;
		}

		constexpr usize PFindFirst(const ValueType* needle, usize needleSize, usize startAt) const noexcept
		{
			if (!needle || needleSize == 0 || startAt >= m_Size) return NoPosition;

			if (needleSize <= 16)
			{
				for (usize i = startAt; i < m_Size; ++i)
					for (usize j = 0; j < needleSize; ++j)
						if (m_Data[i] == needle[j]) return i;
			}
			else
			{
				if constexpr (sizeof(ValueType) == 1)
				{
					bool table[256] = { false };
					for (usize j = 0; j < needleSize; ++j)
						table[static_cast<usize>(needle[j])] = true;

					for (usize i = startAt; i < m_Size; ++i)
						if (table[static_cast<u8>(m_Data[i])])
							return i;
				}
				else if constexpr (IsSame<ValueType, wchar>)
				{
					Bitset<NumericLimit<ValueType>::Max> table;
					for (usize j = 0; j < needleSize; ++j)
						table.Set(static_cast<usize>(needle[j]));
					for (usize i = startAt; i < m_Size; ++i)
						if (table.Test(static_cast<usize>(m_Data[i])))
							return i;
				}
			}

			return NoPosition;
		}

		constexpr usize PFindFirstNot(const ValueType* needle, usize needleSize, usize startAt) const noexcept
		{
			if (!needle || needleSize == 0 || startAt >= m_Size) return NoPosition;

			if (needleSize <= 16)
			{
				for (usize i = startAt; i < m_Size; ++i)
				{
					bool found = false;
					for (usize j = 0; j < needleSize; ++j)
					{
						if (m_Data[i] == needle[j])
						{
							found = true;
							break;
						}
					}
					if (!found)
						return i;
				}
			}
			else
			{
				if constexpr (sizeof(ValueType) == 1)
				{
					bool table[256] = { false };
					for (usize j = 0; j < needleSize; ++j)
						table[static_cast<u8>(needle[j])] = true;

					for (usize i = startAt; i < m_Size; ++i)
						if (!table[static_cast<u8>(m_Data[i])])
							return i;
				}
				else if constexpr (IsSame<ValueType, wchar>)
				{
					Bitset<NumericLimit<ValueType>::Max> table;
					for (usize j = 0; j < needleSize; ++j)
						table.Set(static_cast<usize>(needle[j]));

					for (usize i = startAt; i < m_Size; ++i)
						if (!table.Test(static_cast<usize>(m_Data[i])))
							return i;
				}
			}

			return NoPosition;
		}

		constexpr usize PFindLast(const ValueType* needle, usize needleSize, usize startAt) const noexcept
		{
			if (!needle || needleSize == 0 || startAt >= m_Size) return NoPosition;
			if (startAt >= m_Size) startAt = m_Size - 1;

			if (needleSize <= 16)
			{
				for (usize i = startAt; i != ~0ull; --i)
					for (usize j = 0; j < needleSize; ++j)
						if (m_Data[i] == needle[j]) return i;
			}
			else
			{
				if constexpr (sizeof(ValueType) == 1)
				{
					bool table[256] = { false };
					for (usize j = 0; j < needleSize; ++j)
						table[static_cast<u8>(needle[j])] = true;
					for (usize i = startAt; i != ~0ull; --i)
						if (table[static_cast<u8>(m_Data[i])])
							return i;
				}
				else if constexpr (IsSame<ValueType, wchar>)
				{
					Bitset<NumericLimit<ValueType>::Max> table;
					for (usize j = 0; j < needleSize; ++j)
						table.Set(static_cast<usize>(needle[j]));
					for (usize i = startAt; i != ~0ull; --i)
						if (table.Test(static_cast<usize>(m_Data[i])))
							return i;
				}
			}

			return NoPosition;
		}

		constexpr usize PFindLastNot(const ValueType* needle, usize needleSize, usize startAt) const noexcept
		{
			if (!needle || needleSize == 0 || startAt >= m_Size) return NoPosition;
			if (startAt >= m_Size) startAt = m_Size - 1;

			if (needleSize <= 16)
			{
				for (usize i = startAt; i != ~0ull; --i)
				{
					bool found = false;
					for (usize j = 0; j < needleSize; ++j)
						if (m_Data[i] == needle[j]) { found = true; break; }
					if (!found) return i;
				}
			}
			else
			{
				if constexpr (sizeof(ValueType) == 1)
				{
					bool table[256] = { false };
					for (usize j = 0; j < needleSize; ++j)
						table[static_cast<u8>(needle[j])] = true;
					for (usize i = startAt; i != ~0ull; --i)
						if (!table[static_cast<u8>(m_Data[i])])
							return i;
				}
				else if constexpr (IsSame<ValueType, wchar>)
				{
					Bitset<NumericLimit<ValueType>::Max> table;
					for (usize j = 0; j < needleSize; ++j)
						table.Set(static_cast<usize>(needle[j]));
					for (usize i = startAt; i != ~0ull; --i)
						if (!table.Test(static_cast<usize>(m_Data[i])))
							return i;
				}
			}
			return NoPosition;
		}

	private:
		PtrType m_Data;
		usize m_Size;
	};

	template<Concept::Character CharacterType>
	class Convert<StringViewImpl<CharacterType>>
	{
	public:
		using String = StringViewImpl<CharacterType>;

	public:
		static i8 ToI8(const String& string, usize* position = nullptr, i32 base = 10) noexcept { return static_cast<i8>(ToI64(string, position, base)); }
		static i16 ToI16(const String& string, usize* position = nullptr, i32 base = 10) noexcept { return static_cast<i16>(ToI64(string, position, base)); }
		static i32 ToI32(const String& string, usize* position = nullptr, i32 base = 10) noexcept { return static_cast<i32>(ToI64(string, position, base)); }
		static i64 ToI64(const String& string, usize* position = nullptr, i32 base = 10) noexcept
		{
			if constexpr (sizeof(String::ValueType) == 1)
			{
				char7* endPtr;
				const char7* castedStr = AS_CHAR7(string.String());
				auto result = strtoll(castedStr, &endPtr, base);
				if (position) *position = static_cast<usize>(endPtr - castedStr);
				return result;
			}
			else if constexpr (IsSame<typename String::ValueType, wchar>)
			{
				wchar* endPtr;
				auto result = wcstoll(string.String(), &endPtr, base);
				if (position) *position = static_cast<usize>(endPtr - string.String());
				return result;
			}
			return 0;
		}

		static u8 ToU8(const String& string, usize* position = nullptr, i32 base = 10) noexcept { return static_cast<u8>(ToU64(string, position, base)); }
		static u16 ToU16(const String& string, usize* position = nullptr, i32 base = 10) noexcept { return static_cast<u16>(ToU64(string, position, base)); }
		static u32 ToU32(const String& string, usize* position = nullptr, i32 base = 10) noexcept { return static_cast<u32>(ToU64(string, position, base)); }
		static u64 ToU64(const String& string, usize* position = nullptr, i32 base = 10) noexcept
		{
			if constexpr (sizeof(String::ValueType) == 1)
			{
				char7* endPtr;
				const char7* castedStr = AS_CHAR7(string.String());
				auto result = strtoull(castedStr, &endPtr, base);
				if (position) *position = static_cast<usize>(endPtr - castedStr);
				return result;
			}
			else if constexpr (IsSame<typename String::ValueType, wchar>)
			{
				wchar* endPtr;
				auto result = wcstoull(string.String(), &endPtr, base);
				if (position) *position = static_cast<usize>(endPtr - string.String());
				return result;
			}
			return 0;
		}

		static f32 ToF32(const String& string, usize* position = nullptr) noexcept
		{
			if constexpr (sizeof(String::ValueType) == 1)
			{
				char7* endPtr;
				const char7* castedStr = AS_CHAR7(string.String());
				auto result = strtof(castedStr, &endPtr);
				if (position) *position = static_cast<usize>(endPtr - castedStr);
				return result;
			}
			else if constexpr (IsSame<typename String::ValueType, wchar>)
			{
				wchar* endPtr;
				auto result = wcstof(string.String(), &endPtr);
				if (position) *position = static_cast<usize>(endPtr - string.String());
				return result;
			}
			return 0;
		}

		static f64 ToF64(const String& string, usize* position = nullptr) noexcept
		{
			if constexpr (sizeof(String::ValueType) == 1)
			{
				char7* endPtr;
				const char7* castedStr = AS_CHAR7(string.String());
				auto result = strtod(castedStr, &endPtr);
				if (position) *position = static_cast<usize>(endPtr - castedStr);
				return result;
			}
			else if constexpr (IsSame<typename String::ValueType, wchar>)
			{
				wchar* endPtr;
				auto result = wcstod(string.String(), &endPtr);
				if (position) *position = static_cast<usize>(endPtr - string.String());
				return result;
			}
			return 0;
		}
	};

	typedef StringViewImpl<char8> StringView; // UTF-8
	typedef StringViewImpl<wchar> WideStringView; // UTF-16
}