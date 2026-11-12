#pragma once
#include "Details/ArrayShared.hpp"
#include "MoonCore/Utility/Algorithm.hpp"
#include "MoonCore/Memory/Memory.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "MoonCore/Utility/InitializerList.hpp"
#include "MoonCore/Utility/Nullable.hpp"

namespace MC
{
	template<typename T, usize ArraySize>
		requires IsNothrowDestructible<T>
	class StackArrayImpl
	{
	public:
		using ValueType = T;
		using PtrType = ValueType*;
		using ConstPtrType = const ValueType*;
		using RefType = ValueType&;
		using ConstRefType = const ValueType&;

		using Iterator = ArrayIterator<StackArrayImpl, false>;
		using ConstIterator = ArrayIterator<StackArrayImpl, true>;
		using ReverseIterator = MC::Iterator::ReverseIterator<Iterator>;
		using ConstReverseIterator = MC::Iterator::ReverseIterator<ConstIterator>;


	public:
		constexpr StackArrayImpl() noexcept
			: m_Size(0)
		{}

		constexpr StackArrayImpl(usize size) noexcept
			: m_Size(size)
		{
			MC_CHECK(size <= ArraySize, "Given size > than ArraySize");
			m_Size = size;
			for (usize i = 0; i < m_Size; i++)
				Memory::Construct(&m_Data[i]);
		}

		constexpr StackArrayImpl(const Utility::InitializerList<ValueType>& other) noexcept
			: m_Size(other.size())
		{
			MC_CHECK(m_Size <= ArraySize, "initializer_list overflows StackArrayImpl");
			for (usize i = 0; i < other.size(); i++)
				Memory::Construct(&m_Data[i], other.begin()[i]);
		}

		constexpr StackArrayImpl(const StackArrayImpl& other) noexcept
			: m_Size(other.m_Size)
		{
			for (usize i = 0; i < m_Size; i++)
				Memory::Construct(&m_Data[i], other.m_Data[i]);
		}

		constexpr StackArrayImpl(StackArrayImpl&& other) noexcept
		{
			m_Size = other.m_Size;
			other.m_Size = 0;

			for (usize i = 0; i < m_Size; i++)
				Memory::Construct(&m_Data[i], Move(other.m_Data[i]));
		}

		constexpr ~StackArrayImpl() noexcept
		{
			Clear();
		}

	public:
		constexpr usize Size() const noexcept { return m_Size; }
		static constexpr usize Capacity() noexcept { return ArraySize; }
		constexpr ConstPtrType Data() const noexcept { return m_Data; }
		constexpr PtrType Data() noexcept { return m_Data; }
		constexpr bool Empty() const noexcept { return m_Size <= 0; }

		constexpr RefType Front() noexcept { return *m_Data; }
		constexpr ConstRefType Front() const noexcept { return *m_Data; }
		constexpr RefType Back() noexcept { return *(m_Data + m_Size - 1); }
		constexpr ConstRefType Back() const noexcept { return *(m_Data + m_Size - 1); }

	public:
		constexpr Iterator Begin() noexcept { return Iterator(m_Data); }
		constexpr ConstIterator CBegin() const noexcept { return ConstIterator(m_Data); }
		constexpr Iterator End() noexcept { return Iterator(m_Data + m_Size); }
		constexpr ConstIterator CEnd() const noexcept { return ConstIterator(m_Data + m_Size); }

		constexpr ReverseIterator RBegin() noexcept { return m_Size > 0 ? ReverseIterator(m_Data + m_Size - 1) : REnd(); }
		constexpr ConstReverseIterator CRBegin() const noexcept { return m_Size > 0 ? ConstReverseIterator(m_Data + m_Size - 1) : CREnd(); }
		constexpr ReverseIterator REnd() noexcept { return ReverseIterator(m_Data - 1); }
		constexpr ConstReverseIterator CREnd() const noexcept { return ConstReverseIterator(m_Data - 1); }

	public:
		constexpr Iterator begin() noexcept { return Begin(); }
		constexpr ConstIterator cbegin() const noexcept { return CBegin(); }
		constexpr Iterator end() noexcept { return End(); }
		constexpr ConstIterator cend() const noexcept { return CEnd(); }

		constexpr ReverseIterator rbegin() noexcept { return RBegin(); }
		constexpr ConstReverseIterator crbegin() const noexcept { return CRBegin(); }
		constexpr ReverseIterator rend() noexcept { return REnd(); }
		constexpr ConstReverseIterator crend() const noexcept { return CREnd(); }

	public:
		constexpr StackArrayImpl& operator=(const StackArrayImpl& other) noexcept
		{
			if (this != &other)
			{
				Clear();
				m_Size = other.m_Size;
				for (usize i = 0; i < m_Size; i++)
					Memory::Construct(&m_Data[i], other.m_Data[i]);
			}
			return *this;
		}

		constexpr StackArrayImpl& operator=(StackArrayImpl&& other) noexcept
		{
			if (this != &other)
			{
				Clear();
				m_Size = other.m_Size;
				other.m_Size = 0;

				for (usize i = 0; i < m_Size; i++)
					Memory::Construct(&m_Data[i], Move(other.m_Data[i]));
			}
			return *this;
		}

		constexpr RefType operator[](const usize index) noexcept
		{
			MC_CHECK(index < m_Size, "Index in array is out of range!");
			return m_Data[index];
		}

		constexpr ConstRefType operator[](const usize index) const noexcept
		{
			MC_CHECK(index < m_Size, "Index in array is out of range!");
			return m_Data[index];
		}

		template<Concept::ScopedEnum Enum>
		constexpr RefType operator[](const Enum index) noexcept
		{
			MC_CHECK(static_cast<usize>(index) < m_Size, "Index in array is out of range!");
			return m_Data[static_cast<usize>(index)];
		}

		template<Concept::ScopedEnum Enum>
		constexpr ConstRefType operator[](const Enum index) const noexcept
		{
			MC_CHECK(static_cast<usize>(index) < m_Size, "Index in array is out of range!");
			return m_Data[static_cast<usize>(index)];
		}

	public:
		constexpr void Clear() noexcept
		{
			if constexpr (!IsTriviallyDestructible<T>)
				for (usize i = 0; i < m_Size; ++i) Memory::Destroy(&m_Data[i]);
			m_Size = 0;
		}

		constexpr void Resize(usize size) noexcept
		{
			static_assert(IsMoveConstructible<ValueType>, "Resize requires T to be MoveConstructible!");
			static_assert(IsDefaultConstructible<ValueType>, "Resize requires T to be DefaultConstructible!");

			Resize(size, ValueType());
		}

		constexpr void Resize(usize size, const ValueType& fill) noexcept
		{
			static_assert(IsCopyConstructible<ValueType>, "Resize requires T to be CopyInsertable!");
			MC_CHECK(size <= ArraySize, "Size > ArraySize");

			if (size < m_Size && m_Size < ArraySize)
			{
				for (usize i = size; i < m_Size; ++i) Memory::Destroy(&m_Data[i]);
				m_Size = size;
				return;
			}

			for (ValueType* current = m_Data + m_Size; current != m_Data + size; ++current)
				Memory::Construct(current, fill);
			m_Size = size;
		}

	public:
		constexpr Iterator Insert(ConstIterator position, const ValueType& value) noexcept
		{
			MC_CHECK(m_Size < ArraySize, "StackArrayImpl overflow detected");
			return UncheckedEmplaceImpl(position, value);
		}
		constexpr Iterator Insert(ConstIterator position, ValueType&& value) noexcept
		{
			MC_CHECK(m_Size < ArraySize, "StackArrayImpl overflow detected");
			return UncheckedEmplaceImpl(position, Move(value));
		}
		constexpr Iterator Insert(ConstIterator position, const usize count, const ValueType& value) noexcept
		{
			MC_CHECK(m_Size + count <= ArraySize, "StackArrayImpl overflow detected");
			return UncheckedInsertImpl(position, count, value);
		}
		constexpr Iterator Insert(ConstIterator position, const Utility::InitializerList<ValueType>& list) noexcept
		{
			MC_CHECK(m_Size + list.size() <= ArraySize, "StackArrayImpl overflow detected");
			return UncheckedInsertImpl(position, list);
		}
		template<Concept::InputIteratorType InputIterator> constexpr Iterator Insert(ConstIterator position, InputIterator first, InputIterator last) noexcept
		{
			MC_CHECK(m_Size + last - first <= ArraySize, "StackArrayImpl overflow detected");
			return UncheckedInsertImpl<InputIterator>(position, first, last);
		}

		constexpr Iterator TryInsert(ConstIterator position, const ValueType& value) noexcept
		{
			if (m_Size >= ArraySize) return End();
			return UncheckedEmplaceImpl(position, value);
		}
		constexpr Iterator TryInsert(ConstIterator position, ValueType&& value) noexcept
		{
			if (m_Size >= ArraySize) return End();
			return UncheckedEmplaceImpl(position, Move(value));
		}
		constexpr Iterator TryInsert(ConstIterator position, const usize count, const ValueType& value) noexcept
		{
			if (m_Size + count > ArraySize) return End();
			return UncheckedInsertImpl(position, count, value);
		}
		constexpr Iterator TryInsert(ConstIterator position, const Utility::InitializerList<ValueType>& list) noexcept
		{
			if (m_Size + list.size() > ArraySize) return End();
			return UncheckedInsertImpl(position, list);
		}
		template<Concept::InputIteratorType InputIterator> constexpr Iterator TryInsert(ConstIterator position, InputIterator first, InputIterator last) noexcept
		{
			if (m_Size + last - first > ArraySize) return End();
			return UncheckedInsertImpl<InputIterator>(position, first, last);
		}

		constexpr Iterator UncheckedInsert(ConstIterator position, const ValueType& value) noexcept
		{
			return UncheckedEmplaceImpl(position, value);
		}
		constexpr Iterator UncheckedInsert(ConstIterator position, ValueType&& value) noexcept
		{
			return UncheckedEmplaceImpl(position, Move(value));
		}
		constexpr Iterator UncheckedInsert(ConstIterator position, const usize count, const ValueType& value) noexcept
		{
			return UncheckedInsertImpl(position, count, value);
		}
		constexpr Iterator UncheckedInsert(ConstIterator position, const Utility::InitializerList<ValueType>& list) noexcept
		{
			return UncheckedInsertImpl(position, list);
		}
		template<Concept::InputIteratorType InputIterator> constexpr Iterator UncheckedInsert(ConstIterator position, InputIterator first, InputIterator last) noexcept
		{
			return UncheckedInsertImpl<InputIterator>(position, first, last);
		}

		template<typename... ArgTypes> constexpr Iterator Emplace(ConstIterator it, ArgTypes&&... args) noexcept
		{
			MC_CHECK(m_Size < ArraySize, "StackArrayImpl overflow detected");
			return UncheckedEmplaceImpl(it, Forward<ArgTypes>(args)...);
		}

		template<typename... ArgTypes> constexpr Iterator TryEmplace(ConstIterator it, ArgTypes&&... args) noexcept
		{
			if (m_Size >= ArraySize) return End();
			return UncheckedEmplaceImpl(it, Forward<ArgTypes>(args)...);
		}

		template<typename... ArgTypes> constexpr Iterator UncheckedEmplace(ConstIterator it, ArgTypes&&... args) noexcept
		{
			return UncheckedEmplaceImpl(it, Forward<ArgTypes>(args)...);
		}

		constexpr void PushBack(const ValueType& value) noexcept
		{
			MC_CHECK(m_Size < ArraySize, "StackArrayImpl overflow detected");
			UncheckedPushBackImpl(value);
		}
		constexpr void PushBack(ValueType&& value) noexcept
		{
			MC_CHECK(m_Size < ArraySize, "StackArrayImpl overflow detected");
			UncheckedPushBackImpl(Move(value));
		}

		constexpr bool TryPushBack(const ValueType& value) noexcept
		{
			if (m_Size >= ArraySize) return false;
			UncheckedPushBackImpl(value);
			return true;
		}
		constexpr bool TryPushBack(ValueType&& value) noexcept
		{
			if (m_Size >= ArraySize) return false;
			UncheckedPushBackImpl(Move(value));
			return true;
		}

		constexpr void UncheckedPushBack(const ValueType& value) noexcept
		{
			UncheckedPushBackImpl(value);
		}
		constexpr void UncheckedPushBack(ValueType&& value) noexcept
		{
			UncheckedPushBackImpl(Move(value));
		}

		template<typename... ArgTypes> constexpr RefType EmplaceBack(ArgTypes&&... args) noexcept
		{
			MC_CHECK(m_Size < ArraySize, "StackArrayImpl overflow detected");
			return UncheckedEmplaceBackImpl(Forward<ArgTypes>(args)...);
		}

		template<typename... ArgTypes> constexpr Utility::Nullable<RefType> TryEmplaceBack(ArgTypes&&... args) noexcept
		{
			if (m_Size >= ArraySize) return Utility::Null;
			return Utility::MakeNullable(UncheckedEmplaceBackImpl(Forward<ArgTypes>(args)...));
		}

		template<typename... ArgTypes> constexpr RefType UncheckedEmplaceBack(ArgTypes&&... args) noexcept
		{
			return UncheckedEmplaceBackImpl(Forward<ArgTypes>(args)...);
		}

		constexpr Iterator Erase(Iterator it) noexcept { return EraseImpl(it); }
		constexpr Iterator Erase(ConstIterator it) noexcept { return EraseImpl(it); }
		constexpr Iterator Erase(Iterator first, Iterator last) noexcept { return EraseImpl(first, last); }
		constexpr Iterator Erase(ConstIterator first, ConstIterator last) noexcept { return EraseImpl(first, last); }

		constexpr Iterator EraseAt(usize index) noexcept { return EraseImpl(Begin() + index); }

		constexpr void PopBack() noexcept { PopBackImpl(); }

		static constexpr void Shrink() noexcept {}

	private:
		constexpr void UncheckedPushBackImpl(const ValueType& value) noexcept
		{
			Memory::Construct(&m_Data[m_Size], value);
			m_Size++;
		}

		constexpr void UncheckedPushBackImpl(ValueType&& value) noexcept
		{
			Memory::Construct(&m_Data[m_Size], Move(value));
			m_Size++;
		}

		template <class... val>
		constexpr RefType UncheckedEmplaceBackImpl(val&&... args) noexcept
		{
			PtrType lastLoc = &m_Data[m_Size];

			Memory::Construct(lastLoc, Forward<val>(args)...);
			m_Size++;

			ValueType& result = *lastLoc;

			return result;
		}

		template <class... val>
		constexpr Iterator UncheckedEmplaceImpl(ConstIterator it, val&&... args) noexcept
		{
			usize offset = it - m_Data;

			PtrType itPtr = m_Data + offset;
			PtrType lastLoc = &m_Data[m_Size];

			if (itPtr == lastLoc)
			{
				Memory::Construct(lastLoc, Forward<val>(args)...);
				m_Size++;
			}
			else
			{
				for (PtrType pos = lastLoc; pos > itPtr; --pos)
				{
					Memory::Construct(pos, Move(*(pos - 1)));
					Memory::Destroy(pos - 1);
				}

				Memory::Destroy(itPtr);
				Memory::Construct(itPtr, Forward<val>(args)...);
				m_Size++;
			}

			return Iterator(itPtr);
		}

		constexpr void InsertShift(usize offset, usize count) noexcept
		{
			if (count == 0) return;

			usize newSize = m_Size + count;
			PtrType targetPtr = m_Data + offset;

			usize oldSize = m_Size;

			for (usize i = oldSize; i-- > offset; )
			{
				const usize dest = i + count;
				if (dest >= oldSize)
					Memory::Construct(&m_Data[dest], Move(m_Data[i]));
				else
					m_Data[dest] = Move(m_Data[i]);
			}

			for (usize i = offset; i < offset + count && i < oldSize; ++i)
				Memory::Destroy(&m_Data[i]);

			m_Size = newSize;
		}

		constexpr Iterator UncheckedInsertImpl(ConstIterator position, const usize count, const ValueType& value) noexcept
		{
			usize currentPos = position - CBegin();
			InsertShift(currentPos, count);

			for (usize i = 0; i < count; ++i)
				Memory::Construct(&m_Data[currentPos + i], value);

			return Begin() + currentPos;
		}

		template<Concept::InputIteratorType InputIterator>
		constexpr Iterator UncheckedInsertImpl(ConstIterator position, InputIterator first, InputIterator last) noexcept
		{
			usize currentPos = position - CBegin();
			usize count = last - first;
			InsertShift(currentPos, count);

			for (InputIterator current = first; current != last; ++current)
				Memory::Construct(&m_Data[currentPos + current - first], *current);

			return Begin() + currentPos;
		}

		constexpr Iterator UncheckedInsertImpl(ConstIterator position, const Utility::InitializerList<ValueType>& list) noexcept
		{
			usize currentPos = position - CBegin();
			InsertShift(currentPos, list.size());

			usize i = 0;
			for (auto it = list.begin(); it != list.end(); ++it, ++i)
				Memory::Construct(&m_Data[currentPos + i], *it);

			return Begin() + currentPos;
		}

		constexpr void PopBackImpl() noexcept
		{
			if (m_Size > 0)
			{
				m_Size--;
				Memory::Destroy(&m_Data[m_Size]);
			}
		}

		constexpr Iterator EraseImpl(Iterator it) noexcept
		{
			PtrType wherePtr = it.Ptr();

			for (PtrType pos = wherePtr; pos < m_Data + m_Size - 1; ++pos)
				*pos = Move(*(pos + 1));

			Memory::Destroy(m_Data + m_Size - 1);

			--m_Size;

			return Iterator(wherePtr);
		}

		constexpr Iterator EraseImpl(ConstIterator it) noexcept
		{
			PtrType wherePtr = it.Ptr();

			for (PtrType pos = wherePtr; pos < m_Data + m_Size - 1; ++pos)
				*pos = Move(*(pos + 1));

			Memory::Destroy(m_Data + m_Size - 1);

			--m_Size;

			return Iterator(wherePtr);
		}

		constexpr Iterator EraseImpl(Iterator first, Iterator last) noexcept
		{
			usize pos = first - Begin();
			usize count = last - first;

			for (usize i = pos; i < pos + count; ++i)
				Memory::Destroy(m_Data + i);

			for (usize i = pos + count; i < m_Size; ++i)
				m_Data[pos + (i - (pos + count))] = Move(m_Data[i]);

			m_Size -= count;
			return Begin() + pos;
		}

		constexpr Iterator EraseImpl(ConstIterator first, ConstIterator last) noexcept
		{
			usize pos = first - Begin();
			usize count = last - first;

			for (usize i = pos; i < pos + count; ++i)
				Memory::Destroy(m_Data + i);

			for (usize i = pos + count; i < m_Size; ++i)
				m_Data[pos + (i - (pos + count))] = Move(m_Data[i]);

			m_Size -= count;
			return Begin() + pos;
		}

	private:
		union { ValueType m_Data[ArraySize]; };
		usize m_Size;
	};


	template<typename T, auto ArraySize> using StackArray = StackArrayImpl<T, ToArraySize<ArraySize>()>;
}