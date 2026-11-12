#pragma once

#include "MoonCore/Core.hpp"
#include "MoonCore/Memory/Memory.hpp"
#include "MoonCore/Utility/Algorithm.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "MoonCore/Containers/Bitset.hpp"
#include "StringShared.hpp"

namespace MC
{
	template<Concept::Character CharacterType, Concept::HeapAllocatorType Allocator>
	class StringImpl
	{
	public:
		using ValueType		= CharacterType;
		using PtrType		= ValueType*;
		using ConstPtrType	= const ValueType*;
		using RefType		= ValueType&;
		using ConstRefType	= const ValueType&;

		using AllocatorType = Allocator;

		using Iterator				= StringIterator<CharacterType, false>;
		using ConstIterator			= StringIterator<CharacterType, true>;
		using ReverseIterator		= MC::Iterator::ReverseIterator<Iterator>;
		using ConstReverseIterator	= MC::Iterator::ReverseIterator<ConstIterator>;

	public:
		static constexpr usize NoPosition = NumericLimit<usize>::Max;
		static constexpr usize ResizeMultiplier = 2;
		static constexpr u32 FltPrecision = 6;

	public:
		StringImpl() noexcept
			: m_Data(nullptr), m_Size(0), m_Capacity(10)
		{
			Allocate(m_Capacity);
		}

		StringImpl(usize initialCapacity) noexcept
			: m_Data(nullptr), m_Size(0), m_Capacity(initialCapacity)
	    {
			Allocate(m_Capacity);
		    m_Data[m_Size] = ValueType('\0');
		}

		StringImpl(ValueType str) noexcept
		{
			m_Size = 1;
			m_Capacity = 10;
			m_Data = nullptr;

			Allocate(m_Capacity);
			CopyString(&str, m_Size);
			m_Data[m_Size] = ValueType('\0');
		}

		StringImpl(const ValueType* str) noexcept
		{
			m_Size = GetStringSize(str);
			m_Capacity = m_Size + 1;
			m_Data = nullptr;

			Allocate(m_Capacity);
			CopyString(str, m_Size);
			m_Data[m_Size] = ValueType('\0');
		}

		StringImpl(const ValueType* str, usize size) noexcept
		{
			m_Size = size;
			m_Capacity = m_Size + 1;
			m_Data = nullptr;

			Allocate(m_Capacity);
			CopyString(str, size);
			m_Data[m_Size] = ValueType('\0');
		}

		StringImpl(char7 ch) noexcept
		{
			MC_CHECK(sizeof(ValueType) == 1, "This constructor is available for only UTF8 string!");
			m_Size = 1;
			m_Capacity = 10;
			m_Data = nullptr;

			Allocate(m_Capacity);
			CopyString(reinterpret_cast<const ValueType*>(&ch), m_Size);
			m_Data[m_Size] = ValueType('\0');
		}

		StringImpl(const char7* str) noexcept
		{
			MC_CHECK(sizeof(ValueType) == 1, "This constructor is available for only UTF8 string!");
			m_Size = GetStringSize(reinterpret_cast<const ValueType*>(str));
			m_Capacity = m_Size + 1;
			m_Data = nullptr;

			Allocate(m_Capacity);
			CopyString(reinterpret_cast<const ValueType*>(str), m_Size);
			m_Data[m_Size] = ValueType('\0');
		}

		StringImpl(const char7* str, usize size) noexcept
		{
			MC_CHECK(sizeof(ValueType) == 1, "This constructor is available for only UTF8 string!");
			m_Size = size;
			m_Capacity = m_Size + 1;
			m_Data = nullptr;

			Allocate(m_Capacity);
			CopyString(reinterpret_cast<const ValueType*>(str), size);
			m_Data[m_Size] = ValueType('\0');
		}

		StringImpl(const StringImpl& other) noexcept
		{
			m_Size = other.m_Size;
			m_Capacity = other.m_Capacity;
			m_Data = nullptr;

			Allocate(m_Capacity);
			CopyString(other.m_Data, m_Size);
			m_Data[m_Size] = ValueType('\0');
		}

		StringImpl(StringImpl&& other) noexcept
			: m_Data(other.m_Data), m_Size(other.m_Size), m_Capacity(other.m_Capacity)
		{
			other.m_Size = 0;
			other.m_Capacity = 0;
			other.m_Data = nullptr;
		}

		~StringImpl() noexcept
		{
			if (m_Data)
			{
				AllocatorType::Deallocate(m_Data, m_Capacity * sizeof(ValueType));
				m_Data = nullptr;
			}
		}

	public:
		usize Size() const noexcept { return m_Size; }
		usize Capacity() const noexcept { return m_Capacity; }
		ConstPtrType String() const noexcept { return m_Data; }
		PtrType String() noexcept { return m_Data; }
		bool Empty() const noexcept { return m_Size <= 0; }

		RefType Front() noexcept { return *m_Data; }
		ConstRefType Front() const noexcept { return *m_Data; }
		RefType Back() noexcept { return *(m_Data + m_Size - 1); }
		ConstRefType Back() const noexcept { return *(m_Data + m_Size - 1); }

	public:
		Iterator Begin() const noexcept { return Iterator(m_Data); }
		ConstIterator CBegin() const noexcept { return ConstIterator(m_Data); }
		Iterator End() const noexcept { return Iterator(m_Data + m_Size); }
		ConstIterator CEnd() const noexcept { return ConstIterator(m_Data + m_Size); }

		ReverseIterator RBegin() const noexcept { return m_Size > 0 ? ReverseIterator(m_Data + m_Size - 1) : REnd(); }
		ConstReverseIterator CRBegin() const noexcept { return m_Size > 0 ? ConstReverseIterator(m_Data + m_Size - 1) : CREnd(); }
		ReverseIterator REnd() const noexcept { return ReverseIterator(m_Data - 1); }
		ConstReverseIterator CREnd() const noexcept { return ConstReverseIterator(m_Data - 1); }

	public:
		Iterator begin() const noexcept { return Begin(); }
		ConstIterator cbegin() const noexcept { return CBegin(); }

		Iterator end() const noexcept { return End(); }
		ConstIterator cend() const noexcept { return CEnd(); }

		ReverseIterator rbegin() const noexcept { return RBegin(); }
		ConstReverseIterator crbegin() const noexcept { return CRBegin(); }

		ReverseIterator rend() const noexcept { return REnd(); }
		ConstReverseIterator crend() const noexcept { return CREnd(); }

	public:
		StringViewImpl<CharacterType> ToStringView() const noexcept
		{
			return StringViewImpl<CharacterType>(m_Data, m_Size);
		}

		operator StringViewImpl<ValueType>() const noexcept
		{
			return StringViewImpl<ValueType>(m_Data, m_Size);
		}

	public:
		StringImpl& operator=(const ValueType* str) noexcept
		{
			Clear();
			Assign(str);
			return *this;
		}

		StringImpl& operator=(const ValueType& str) noexcept
		{
			Clear();
			AddChar(str);
			return *this;
		}

		StringImpl& operator=(const StringImpl& other) noexcept
		{
			if (this != &other)
			{
				if (m_Data) AllocatorType::Deallocate(m_Data, m_Capacity);
				m_Size = other.m_Size;
				m_Capacity = other.m_Capacity;
				m_Data = nullptr;

				Allocate(m_Capacity);
				CopyString(other.m_Data, m_Size);
				m_Data[m_Size] = ValueType('\0');
			}
			return *this;
		}

		StringImpl& operator=(StringImpl&& other) noexcept 
		{
			if (this != &other)
			{
				if (m_Data)
					AllocatorType::Deallocate(m_Data, m_Capacity * sizeof(ValueType));
				m_Data = other.m_Data;
				m_Size = other.m_Size;
				m_Capacity = other.m_Capacity;

				other.m_Data = nullptr;
				other.m_Size = 0;
				other.m_Capacity = 0;
			}
			return *this;
		}

		bool operator==(const StringImpl& str) const noexcept
		{
			if (str.Size() != m_Size) return false;

			if constexpr (IsTriviallyCopyable<ValueType>)
			{
				return memcmp(m_Data, str.m_Data, m_Size * sizeof(ValueType)) == 0;
			}
			else
			{
				for (usize i = 0; i < m_Size; i++)
					if (m_Data[i] != str.m_Data[i])
						return false;
				return true;
			}
		}

		bool operator==(const StringViewImpl<CharacterType>& str) const noexcept
		{
			if (str.Size() != m_Size) return false;

			if constexpr (IsTriviallyCopyable<ValueType>)
			{
				return memcmp(m_Data, str.String(), m_Size * sizeof(ValueType)) == 0;
			}
			else
			{
				for (usize i = 0; i < m_Size; i++)
					if (m_Data[i] != str.String()[i])
						return false;
				return true;
			}
		}

		StringImpl operator+(const ValueType* str) const noexcept
		{
			StringImpl result = *this;
			result.AddToString(str, GetStringSize(str));
			return result;
		}

		StringImpl operator+(const StringImpl& str) const noexcept
		{
			StringImpl result = *this;
			result.AddToString(str.String(), str.Size());
			return result;
		}

		StringImpl operator+(StringViewImpl<ValueType> str) const noexcept
		{
			StringImpl result = *this;
			result.AddToString(str.String(), str.Size());
			return result;
		}

		StringImpl operator+(const ValueType& character) const noexcept
		{
			StringImpl result = *this;
			result.AddChar(character);
			return result;
		}

		StringImpl& operator+=(const StringImpl& str) noexcept
		{
			return AddToString(str.String(), str.Size());
		}

		StringImpl operator+=(StringViewImpl<ValueType> str) noexcept
		{
			return AddToString(str.String(), str.Size());
		}

		StringImpl& operator+=(const ValueType* str) noexcept
		{
			return AddToString(str, GetStringSize(str));
		}

		StringImpl& operator+=(const ValueType& character) noexcept
		{
			return AddChar(character);
		}

		RefType operator[](const usize index) noexcept
		{
			MC_CHECK(index < m_Size, "Index in string is out of range");
			return m_Data[index];
		}

		ConstRefType operator[](const usize index) const noexcept
		{
			MC_CHECK(index < m_Size, "Index in string is out of range");
			return m_Data[index];
		}

	public:
		void Clear()
		{
			m_Size = 0;
			if (m_Data)
				m_Data[0] = ValueType('\0');
		}

		Iterator Insert(ConstIterator position, ValueType character)
		{
			return PInsert(position, 1, character);
		}

		Iterator Insert(ConstIterator position, const usize count, ValueType character)
		{
			return PInsert(position, count, character);
		}

		template<typename InputIt>
		Iterator Insert(ConstIterator position, InputIt first, InputIt last)
		{
			return PInsert<InputIt>(position, first, last);
		}

		// TODO: initializer_list replacement
		//Iterator Insert(ConstIterator position, Utility::InitializerList<ValueType> characters)
		//{
		//	return PInsert(position, characters);
		//}

		void PushBack(ValueType character)
		{
			AddChar(character);
		}

		void PopBack()
		{
			if (m_Size > 0)
			{
				m_Size--;
				m_Data[m_Size] = ValueType('\0');
			}
		}

		bool Reserve(usize size)
		{
			if (size <= m_Capacity) return false;
			Allocate(size);
			return true;
		}

		void Resize(usize size)
		{
			Resize(size, ValueType());
		}

		void Resize(usize size, ValueType fill)
		{
			if (size < m_Size && m_Size < m_Capacity)
			{
				m_Size = size;
				m_Data[m_Size] = ValueType('\0');
				return;
			}

			if (size >= m_Capacity) Allocate(size + 1);
			m_Data[size] = ValueType('\0');
			memset(m_Data + m_Size, static_cast<i32>(fill), size - m_Size);
			m_Size = size;
		}

	public:
		CodepointRange<CharacterType, true>::Iterator CodepointBegin() const noexcept
		{
			return CodepointRange<CharacterType, true>::Iterator(m_Data);
		}

		CodepointRange<CharacterType, true>::Iterator CodepointEnd() const noexcept
		{
			return CodepointRange<CharacterType, true>::Iterator(m_Data + m_Size);
		}

		CodepointRange<CharacterType, true> Codepoints() const noexcept
		{
			return CodepointRange<CharacterType, true>(m_Data, m_Data + m_Size);
		}

		usize CodepointCount() const noexcept
		{
			usize count = 0;
			for (usize i = 0; i < m_Size; ++i)
				if ((static_cast<u8>(m_Data[i]) & 0xC0) != 0x80)
					++count;
			return count;
		}

		char32 GetCodepoint(usize codepointIndex) const noexcept
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

		usize CodepointOffset(usize codepointIndex) const noexcept
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

		StringImpl CodepointSubstring(usize startCodepoint, usize codepointCount) const noexcept
		{
			usize startByte = CodepointOffset(startCodepoint);
			auto it = CodepointRange<CharacterType, true>::Iterator(m_Data + startByte);
			const auto e = CodepointEnd();
			for (usize i = 0; i < codepointCount && it != e; ++i)
				++it;
			usize endByte = static_cast<usize>(it.Ptr() - m_Data);
			return StringImpl(m_Data + startByte, endByte - startByte);
		}

	public:
		StringImpl Substring(usize start, usize length = NoPosition) const noexcept
		{
			MC_CHECK(start < m_Size, "Start index is out of range");
			usize remaining = m_Size - start;
			usize newSize = (length == NoPosition || length > remaining) ? remaining : length;
			return StringImpl(m_Data + start, newSize);
		}

		usize Find(const StringImpl& in, usize pos = 0) const noexcept
		{
			return PFind(in.String(), in.Size(), pos);
		}

		usize Find(const ValueType* in, usize pos = 0) const noexcept
		{
			return PFind(in, GetStringSize(in), pos);
		}

		usize Find(const ValueType* in, usize pos, usize n) const noexcept
		{
			return PFind(in, n, pos);
		}

		usize Find(ValueType in, usize pos = 0) const noexcept
		{
			return PFind(const_cast<const ValueType*>(&in), 1, pos);
		}

		usize FindFirst(const StringImpl& in, usize pos = 0) const noexcept
		{
			return PFindFirst(in.String(), in.Size(), pos);
		}

		usize FindFirst(const ValueType* in, usize pos = 0) const noexcept
		{
			return PFindFirst(in, GetStringSize(in), pos);
		}

		usize FindFirst(const ValueType* in, usize pos, usize n) const noexcept
		{
			return PFindFirst(in, n, pos);
		}

		usize FindFirst(ValueType in, usize pos = 0) const noexcept
		{
			return PFindFirst(const_cast<const ValueType*>(&in), 1, pos);
		}


		usize FindFirstNot(const StringImpl& in, usize pos = 0) const noexcept
		{
			return PFindFirstNot(in.String(), in.Size(), pos);
		}

		usize FindFirstNot(const ValueType* in, usize pos = 0) const noexcept
		{
			return PFindFirstNot(in, GetStringSize(in), pos);
		}

		usize FindFirstNot(const ValueType* in, usize pos, usize n) const noexcept
		{
			return PFindFirstNot(in, n, pos);
		}

		usize FindFirstNot(ValueType in, usize pos = 0) const noexcept
		{
			return PFindFirstNot(const_cast<const ValueType*>(&in), 1, pos);
		}


		usize FindLast(const StringImpl& in, usize pos = NoPosition) const noexcept
		{
			if (pos == NoPosition) pos = m_Size - 1;
			return PFindLast(in.String(), in.Size(), pos);
		}

		usize FindLast(const ValueType* in, usize pos = NoPosition) const noexcept
		{
			if (pos == NoPosition) pos = m_Size - 1;
			return PFindLast(in, GetStringSize(in), pos);
		}

		usize FindLast(const ValueType* in, usize pos, usize n) const noexcept
		{
			if (pos == NoPosition) pos = m_Size - 1;
			return PFindLast(in, n, pos);
		}

		usize FindLast(ValueType in, usize pos = NoPosition) const noexcept
		{
			if (pos == NoPosition) pos = m_Size - 1;
			return PFindLast(const_cast<const ValueType*>(&in), 1, pos);
		}


		usize FindLastNot(const StringImpl& in, usize pos = NoPosition) const noexcept
		{
			if (pos == NoPosition) pos = m_Size - 1;
			return PFindLastNot(in.String(), in.Size(), pos);
		}

		usize FindLastNot(const ValueType* in, usize pos = NoPosition) const noexcept
		{
			if (pos == NoPosition) pos = m_Size - 1;
			return PFindLastNot(in, GetStringSize(in), pos);
		}

		usize FindLastNot(const ValueType* in, usize pos, usize n) const noexcept
		{
			if (pos == NoPosition) pos = m_Size - 1;
			return PFindLastNot(in, n, pos);
		}

		usize FindLastNot(ValueType in, usize pos = NoPosition) const noexcept
		{
			if (pos == NoPosition) pos = m_Size - 1;
			return PFindLastNot(const_cast<const ValueType*>(&in), 1, pos);
		}

	public:
		void Assign(const ValueType*& ptr) noexcept
		{
			Assign(ptr, GetStringSize(ptr));
		}

		void Assign(const ValueType*& ptr, usize size) noexcept
		{
			MC_CHECK(ptr != nullptr, "Assigning nullptr to string");
			if (size + 1 > m_Capacity)
				Allocate(size + 1);

			CopyString(ptr, size);
			m_Size = size;
			m_Data[m_Size] = ValueType('\0');
		}

	public:
		static StringImpl ToString(i8 value) noexcept { return ToString(static_cast<i64>(value)); }
		static StringImpl ToString(i16 value) noexcept { return ToString(static_cast<i64>(value)); }
		static StringImpl ToString(i32 value) noexcept { return ToString(static_cast<i64>(value)); }
		static StringImpl ToString(i64 value) noexcept
		{
			if constexpr (sizeof(ValueType) == 1)
			{
				if (value == 0) return TEXT("0");

				bool negative = value < 0;
				if (negative) value = -value;

				StringImpl result;
				while (value > 0)
				{
					result.Insert(result.CBegin(), static_cast<char8>(TEXT('0') + (value % 10)));
					value /= 10;
				}

				if (negative) result.Insert(result.CBegin(), TEXT('-'));
				return result;
			}
			else if constexpr (sizeof(ValueType) == 2)
			{
				if (value == 0) return TEXTW("0");

				bool negative = value < 0;
				if (negative) value = -value;

				StringImpl result;
				while (value > 0)
				{
					result.Insert(result.CBegin(), static_cast<wchar>(TEXTW('0') + (value % 10)));
					value /= 10;
				}

				if (negative) result.Insert(result.CBegin(), TEXTW('-'));
				return result;
			}

			return "";
		}

		static StringImpl ToString(u8 value) noexcept { return ToString(static_cast<u64>(value)); }
		static StringImpl ToString(u16 value) noexcept { return ToString(static_cast<u64>(value)); }
		static StringImpl ToString(u32 value) noexcept { return ToString(static_cast<u64>(value)); }
		static StringImpl ToString(u64 value) noexcept
		{

			if constexpr (sizeof(ValueType) == 1)
			{
				if (value == 0) return TEXT("0");

				StringImpl result;
				while (value > 0)
				{
					result.Insert(result.CBegin(), static_cast<char8>(TEXT('0') + (value % 10)));
					value /= 10;
				}
				return result;
			}
			else if constexpr (sizeof(ValueType) == 2)
			{
				if (value == 0) return TEXTW("0");

				StringImpl result;
				while (value > 0)
				{
					result.Insert(result.CBegin(), static_cast<wchar>(TEXTW('0') + (value % 10)));
					value /= 10;
				}
				return result;
			}

			return "";
		}

		static StringImpl ToString(f32 value) noexcept
		{
			if constexpr (sizeof(ValueType) == 1)
			{
				if (-NumericLimit<f32>::Epsilon <= value && value <= NumericLimit<f32>::Epsilon) return TEXT("0");
			}
			else if constexpr (sizeof(ValueType) == 2)
			{
				if (-NumericLimit<f32>::Epsilon <= value && value <= NumericLimit<f32>::Epsilon) return TEXTW("0");
			}
			return ToString(static_cast<f64>(value));
		}

		static StringImpl ToString(f64 value) noexcept
		{
			if constexpr (sizeof(ValueType) == 1)
			{
				if (-NumericLimit<f64>::Epsilon <= value && value <= NumericLimit<f64>::Epsilon) return TEXT("0");

				bool negative = value < 0;
				if (negative) value = -value;

				i64 iPart = static_cast<i64>(value);
				f64 fracPart = value - static_cast<f64>(iPart);

				StringImpl result = ToString(iPart);
				result.PushBack(TEXT('.'));

				for (i32 i = 0; i < FltPrecision && fracPart > 0; i++)
				{
					fracPart *= 10;
					i32 digit = static_cast<i32>(fracPart);
					result.PushBack(static_cast<char8>(TEXT('0') + digit));
					fracPart -= digit;
				}

				while (result.Size() > 1 && result[result.Size() - 1] == TEXT('0'))
					result.PopBack();
				if (result[result.Size() - 1] == TEXT('.'))
					result.PopBack();

				if (negative) result.Insert(result.CBegin(), TEXT('-'));
				return result;
			}
			else if constexpr (sizeof(ValueType) == 2)
			{
				if (value <= NumericLimit<f64>::Epsilon) return TEXTW("0");

				bool negative = value < 0;
				if (negative) value = -value;

				i64 iPart = static_cast<i64>(value);
				f64 fracPart = value - static_cast<f64>(iPart);

				StringImpl result = ToString(iPart);
				result.PushBack(TEXTW('.'));

				for (i32 i = 0; i < FltPrecision && fracPart > 0; i++)
				{
					fracPart *= 10;
					i32 digit = static_cast<i32>(fracPart);
					result.PushBack(static_cast<char8>(TEXTW('0') + digit));
					fracPart -= digit;
				}

				while (result.Size() > 1 && result[result.Size() - 1] == TEXTW('0'))
					result.PopBack();
				if (result[result.Size() - 1] == TEXTW('.'))
					result.PopBack();

				if (negative) result.Insert(result.CBegin(), TEXTW('-'));
				return result;
			}

			return "";
		}

		static StringImpl ToString(bool value) noexcept
		{
			if constexpr (sizeof(ValueType) == 1)
			{
				return value ? TEXT("true") : TEXT("false");
			}
			else if constexpr (sizeof(ValueType) == 2)
			{
				return value ? TEXTW("true") : TEXTW("false");
			}

			return "";
		}

		static StringImpl ToString(char8 value) noexcept
		{
			return StringImpl(value);
		}

		static StringImpl ToString(const StringImpl& value) noexcept
		{
			return value;
		}

	private:
		void Allocate(usize newCapacity) noexcept
		{
			PtrType newBlock = AllocatorType::Allocate(newCapacity);
			usize copySize = m_Size < newCapacity - 1 ? m_Size : newCapacity - 1;

			if (m_Data)
			{
				for (usize i = 0; i < copySize; i++)
					newBlock[i] = m_Data[i];
				AllocatorType::Deallocate(m_Data, m_Capacity);
			}

			m_Data = newBlock;
			m_Capacity = newCapacity;
			m_Size = Min(copySize, m_Size);
			m_Data[m_Size] = '\0';
		}

		void CopyString(const ValueType* str, usize size) noexcept
		{
			memcpy(m_Data, str, size * sizeof(ValueType));
		}

		StringImpl& AddToString(const ValueType* str, usize size) noexcept
		{
			if (m_Size + size + 1 > m_Capacity)
				Allocate((m_Size + size + 1) * ResizeMultiplier);

			for (usize i = 0; i < size; i++)
				m_Data[m_Size + i] = str[i];

			m_Size += size;
			m_Data[m_Size] = '\0';
			return *this;
		}

		StringImpl& AddChar(const ValueType& character) noexcept
		{
			if (m_Size + 1 >= m_Capacity)
				Allocate(m_Capacity * ResizeMultiplier);

			if (m_Size == 0)
			{
				m_Data[0] = character;
				m_Size++;
				m_Data[m_Size] = '\0';
				return *this;
			}

			m_Data[m_Size] = character;
			m_Size++;
			m_Data[m_Size] = '\0';
			return *this;
		}

		Iterator PInsert(ConstIterator position, const usize count, const ValueType& value) noexcept
		{
			usize currentPos = static_cast<usize>(position - CBegin());
			if (m_Size + count + 1 > m_Capacity)
				Allocate((m_Size + count + 1) * ResizeMultiplier);
			for (usize i = m_Size; i > currentPos; --i)
				m_Data[i + count - 1] = m_Data[i - 1];
			for (usize i = 0; i < count; ++i)
				m_Data[currentPos + i] = value;
			m_Size += count;
			m_Data[m_Size] = ValueType('\0');
			return Begin() + currentPos;
		}

		template<typename InputIt>
		Iterator PInsert(ConstIterator position, InputIt first, InputIt last) noexcept
		{
			usize count = last - first;
			usize currentPos = static_cast<usize>(position - CBegin());
			if (m_Size + count + 1 > m_Capacity)
				Allocate((m_Size + count + 1) * ResizeMultiplier);
			for (usize i = m_Size; i > currentPos; --i)
				m_Data[i + count - 1] = m_Data[i - 1];
			for (usize i = 0; i < count; ++i)
				m_Data[currentPos + i] = *(first + i);
			m_Size += count;
			m_Data[m_Size] = ValueType('\0');
			return Begin() + currentPos;
		}

		// TODO:: initializer_list replacement
		//Iterator PInsert(ConstIterator position, const Utility::InitializerList<ValueType>& list) noexcept
		//{
		//	usize count = list.size();
		//	usize currentPos = static_cast<usize>(position - CBegin());
		//	if (m_Size + count + 1 > m_Capacity)
		//		Allocate((m_Size + count + 1) * RESIZE_MULTIPLIER);
		//	for (usize i = m_Size; i > currentPos; --i)
		//		m_Data[i + count - 1] = m_Data[i - 1];
		//	for (usize i = 0; i < count; ++i)
		//		m_Data[currentPos + i] = *(list.begin() + i);
		//	m_Size += count;
		//	m_Data[m_Size] = ValueType('\0');
		//	return Begin() + currentPos;
		//}

	private:
		usize PFind(const ValueType* needle, usize needleSize, usize startAt) const noexcept
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

		usize PFindFirst(const ValueType* needle, usize needleSize, usize startAt) const noexcept
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

		usize PFindFirstNot(const ValueType* needle, usize needleSize, usize startAt) const noexcept
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

		usize PFindLast(const ValueType* needle, usize needleSize, usize startAt) const noexcept
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

		usize PFindLastNot(const ValueType* needle, usize needleSize, usize startAt) const noexcept
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
		usize m_Capacity;
	};

	template<Concept::Character CharacterType, typename Allocator>
	class Convert<StringImpl<CharacterType, Allocator>>
	{
	public:
		using String = StringImpl<CharacterType, Allocator>;

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
				u64 result = strtoll(castedStr, &endPtr, base);
				if (position) *position = static_cast<usize>(endPtr - castedStr);
				return result;
			}
			else if constexpr (IsSame<typename String::ValueType, wchar>)
			{
				wchar* endPtr;
				i64 result = wcstoll(string.String(), &endPtr, base);
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

	typedef StringImpl<char8> UTF8String;
	typedef StringImpl<wchar> WideString;
	typedef UTF8String String;	

	inline String operator+(const char7* str1, const String& str2) noexcept { return String::ToString(str1) += str2.String(); }
	inline String operator+(const char8* str1, const String& str2) noexcept { return String::ToString(str1) += str2.String(); }
	inline String operator+(char7 character, const String& str2) noexcept { return String::ToString(character) += str2.String(); }
	inline String operator+(char8 character, const String& str2) noexcept { return String::ToString(character) += str2.String(); }
	inline WideString operator+(const wchar* str1, const WideString& str2) noexcept { return WideString::ToString(str1) += str2.String(); }
	inline WideString operator+(wchar character, const WideString& str2) noexcept { return WideString::ToString(character) += str2.String(); }

	inline String WideStringToString(const WideString& str) noexcept
	{
		usize num;
		usize strSize = str.Size() + 1;
		char7* resultStr = new char7[strSize];

		errno_t err = wcstombs_s(&num, resultStr, strSize, str.String(), str.Size());
		if (err != 0)
		{
			delete[] resultStr;
			return "";
		}

		String result = String(resultStr, num - 1);
		delete[] resultStr;
		return result;
	}

	inline WideString StringToWideString(const String& str) noexcept
	{
		usize num;
		usize strSize = str.Size() + 1;
		wchar* resultStr = new wchar[strSize];
		errno_t err = mbstowcs_s(&num, resultStr, strSize, AS_CHAR7(str.String()), str.Size());

		if (err != 0)
		{
			delete[] resultStr;
			return L"";
		}

		WideString result = WideString(resultStr, num - 1);
		delete[] resultStr;
		return result;
	}
}