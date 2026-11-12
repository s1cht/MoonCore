#pragma once
#include "Details/ArrayShared.hpp"
#include "MoonCore/Utility/Algorithm.hpp"
#include "MoonCore/Memory/Memory.hpp"
#include "MoonCore/Memory/Allocators/Heap/HeapAllocator.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "MoonCore/Utility/InitializerList.hpp"

namespace MC
{
	template<typename T, 
        Concept::HeapAllocatorType Allocator = Memory::HeapAllocator<T>>
	    requires IsNothrowDestructible<T>
    class Array
	{
	public:
		using ValueType		= T;
		using PtrType		= ValueType*;
		using ConstPtrType	= const ValueType*;
		using RefType		= ValueType&;
		using ConstRefType	= const ValueType&;

		using AllocatorType = Allocator;

		using Iterator				= ArrayIterator<Array, false>;
		using ConstIterator			= ArrayIterator<Array, true>;
		using ReverseIterator		= MC::Iterator::ReverseIterator<Iterator>;
		using ConstReverseIterator	= MC::Iterator::ReverseIterator<ConstIterator>;

	private:
	    static constexpr u8 ARR_RESIZE_MULTIPLIER = 2;

	public:
		Array() noexcept
			: m_Data(nullptr), m_Size(0), m_Capacity(20), m_Allocator(AllocatorType())
		{
			Allocate(m_Capacity);
		}

		Array(usize size) noexcept
			: m_Data(nullptr), m_Size(size), m_Capacity(size), m_Allocator(AllocatorType())
		{
			Allocate(m_Capacity);
			m_Size = size;
			for (usize i = 0; i < m_Size; i++)
				Memory::Construct(&m_Data[i]);
		}

		Array(const Array& other) noexcept
		{
			m_Allocator = other.m_Allocator;
			m_Capacity = other.m_Capacity;
			m_Data = nullptr;

			Allocate(m_Capacity);

			m_Size = other.m_Size;
			for (usize i = 0; i < m_Size; i++)
				Memory::Construct(&m_Data[i], other.m_Data[i]);
		}

		Array(const Utility::InitializerList<ValueType>& other) noexcept
			: m_Data(nullptr), m_Allocator(AllocatorType())
		{
			m_Data = nullptr;
			m_Size = 0;

			Allocate(other.size() > 20 ? other.size() * ARR_RESIZE_MULTIPLIER : 20);

			m_Size = other.size();
			for (usize i = 0; i < other.size(); i++)
				Memory::Construct(&m_Data[i], other.begin()[i]);
		}

		Array(Array&& other) noexcept
		{
			m_Allocator = other.m_Allocator;
			m_Size = other.m_Size;
			m_Capacity = other.m_Capacity;

			other.m_Size = 0;
			other.m_Capacity = 0;

			m_Data = other.m_Data;
			other.m_Data = nullptr;
		}

		~Array() noexcept
		{
			Clear();

			if (m_Data)
			{
				m_Allocator.Deallocate(m_Data, m_Capacity);
				m_Data = nullptr;
			}
		}

	public:
		usize Size() const noexcept { return m_Size; }
		usize Capacity() const noexcept { return m_Capacity; }
		ConstPtrType Data() const noexcept { return m_Data; }
		PtrType Data() noexcept { return m_Data; }
		bool Empty() const noexcept { return m_Size <= 0; }

		RefType Front() noexcept { return *m_Data; }
		ConstRefType Front() const noexcept { return *m_Data; }
		RefType Back() noexcept { return *(m_Data + m_Size - 1); }
		ConstRefType Back() const noexcept { return *(m_Data + m_Size - 1); }

	public:
		Iterator Begin() noexcept { return Iterator(m_Data); }
		ConstIterator Begin() const noexcept { return ConstIterator(m_Data); }
		ConstIterator CBegin() const noexcept { return ConstIterator(m_Data); }
		Iterator End() noexcept { return Iterator(m_Data + m_Size); }
		ConstIterator End() const noexcept { return ConstIterator(m_Data + m_Size); }
		ConstIterator CEnd() const noexcept { return ConstIterator(m_Data + m_Size); }

		ReverseIterator RBegin() noexcept { return m_Size > 0 ? ReverseIterator(m_Data + m_Size - 1) : REnd(); }
		ConstReverseIterator RBegin() const noexcept { return m_Size > 0 ? ConstReverseIterator(m_Data + m_Size - 1) : REnd(); }
		ConstReverseIterator CRBegin() const noexcept { return m_Size > 0 ? ConstReverseIterator(m_Data + m_Size - 1) : CREnd(); }
		ReverseIterator REnd() noexcept { return ReverseIterator(m_Data - 1); }
		ConstReverseIterator REnd() const noexcept { return ConstReverseIterator(m_Data - 1); }
		ConstReverseIterator CREnd() const noexcept { return ConstReverseIterator(m_Data - 1); }

	public:
		Iterator begin() noexcept { return Begin(); }
		ConstIterator begin() const noexcept { return Begin(); }
		ConstIterator cbegin() const noexcept { return CBegin(); }
		Iterator end() noexcept { return End(); }
		ConstIterator end() const noexcept { return End(); }
		ConstIterator cend() const noexcept { return CEnd(); }

		ReverseIterator rbegin() noexcept { return RBegin(); }
		ConstReverseIterator rbegin() const noexcept { return RBegin(); }
		ConstReverseIterator crbegin() const noexcept { return CRBegin(); }
		ReverseIterator rend() noexcept { return REnd(); }
		ConstReverseIterator rend() const noexcept { return REnd(); }
		ConstReverseIterator crend() const noexcept { return CREnd(); }

	public:
		Array& operator=(const Array& other) noexcept
		{
			if (this != &other)
			{
				if (m_Data) m_Allocator.Deallocate(m_Data, m_Capacity);
                m_Allocator = other.m_Allocator;
				m_Capacity = other.m_Capacity;
				m_Data = nullptr;

                Allocate(m_Capacity);

				m_Size = other.m_Size;
				for (usize i = 0; i < m_Size; i++) Memory::Construct(&m_Data[i], other.m_Data[i]);
			}
			return *this;
		}

		Array& operator=(Array&& other) noexcept
		{
			if (this != &other)
			{
				if (m_Data) m_Allocator.Deallocate(m_Data, m_Capacity);
				m_Data = other.m_Data;
				m_Size = other.m_Size;
				m_Capacity = other.m_Capacity;
				m_Allocator = other.m_Allocator;

				other.m_Data = nullptr;
				other.m_Size = 0;
				other.m_Capacity = 0;
			}
			return *this;
		}

		RefType operator[](const usize index) noexcept
		{
			MC_CHECK(index < m_Size, "Index in array is out of range!");
			return m_Data[index];
		}

		ConstRefType operator[](const usize index) const noexcept
		{
			MC_CHECK(index < m_Size, "Index in array is out of range!");
			return m_Data[index];
		}

	public:
		void Clear() noexcept
		{
			if constexpr (!IsTriviallyDestructible<T>)
				for (usize i = 0; i < m_Size; ++i) Memory::Destroy(&m_Data[i]);
			m_Size = 0;
		}

		void Append(const Array& other) noexcept { PAppend(other); }
		void Append(Array&& other) noexcept { PAppend(Move(other)); }
		void Shrink() noexcept { PShrink(); }

		bool Reserve(usize size) noexcept { return PReserve(size); }
		void Resize(usize size, const ValueType& fill) noexcept
		{
			static_assert(IsCopyConstructible<ValueType>, "Resize requires T to be CopyConstructible!");

			if (size < m_Size && m_Size < m_Capacity) { m_Size = size; return; }
			if (size >= m_Capacity) Allocate(size + 1);

			for (ValueType* current = m_Data + m_Size; current != m_Data + size; ++current)
				Memory::Construct(current, fill);
			m_Size = size;
		}

		template<typename... ArgTypes>
		void Resize(usize size, ArgTypes&&... args) noexcept
		{
			static_assert(IsMoveConstructible<ValueType>, "Resize requires T to be MoveConstructible!");

			if (size < m_Size && m_Size < m_Capacity) { m_Size = size; return; }
			if (size >= m_Capacity) Allocate(size + 1);

			for (ValueType* current = m_Data + m_Size; current != m_Data + size; ++current)
				Memory::Construct(current, T(args...));
			m_Size = size;
		}

	public:
		Iterator Insert(ConstIterator position, const ValueType& value) noexcept { return PEmplace(position, value); }
		Iterator Insert(ConstIterator position, ValueType&& value) noexcept { return PEmplace(position, Move(value)); }
		Iterator Insert(ConstIterator position, const usize count, const ValueType& value) noexcept { return PInsert(position, count, value); }
		Iterator Insert(ConstIterator position, const Utility::InitializerList<ValueType>& list) noexcept { return PInsert(position, list); }
		template<Concept::InputIteratorType InputIterator> Iterator Insert(ConstIterator position, InputIterator first, InputIterator last) noexcept { return PInsert<InputIterator>(position, first, last); }

		template<typename... ArgTypes> Iterator Emplace(ConstIterator it, ArgTypes&&... args) noexcept { return PEmplace(it, Forward<ArgTypes>(args)...); }

		void PushBack(const ValueType& value) noexcept { PPushBack(value); }
		void PushBack(ValueType&& value) noexcept { PPushBack(Move(value)); }

		template<typename... ArgTypes> RefType EmplaceBack(ArgTypes&&... args) noexcept { return PEmplaceBack(Forward<ArgTypes>(args)...); }

		Iterator Erase(Iterator it) noexcept { return PErase(it); }
		Iterator Erase(ConstIterator it) noexcept { return PErase(it); }
		Iterator Erase(Iterator first, Iterator last) noexcept { return PErase(first, last); }
		Iterator Erase(ConstIterator first, ConstIterator last) noexcept { return PErase(first, last); }

		Iterator EraseAt(usize index) noexcept { return PErase(Begin() + index); }

		void PopBack() noexcept { PPopBack(); }

	private:
		void Allocate(usize newCapacity) noexcept
		{
			ValueType* newBlock = m_Allocator.Allocate(newCapacity);

			usize elementsToMove = (m_Data && m_Capacity > 0) ? Min(m_Size, newCapacity) : 0;

			if (m_Data && m_Capacity > 0)
			{
				    if constexpr (IsTriviallyCopyable<ValueType>)
				    {
					    memmove(newBlock, m_Data, elementsToMove * sizeof(ValueType));
				    }
				    else
				    {
					    for (usize i = 0; i < elementsToMove; i++)
					    {
							Memory::Construct(&newBlock[i], Move(m_Data[i]));
							Memory::Destroy(&m_Data[i]);
					    }
				    }

				m_Allocator.Deallocate(m_Data, m_Capacity);
			}

			m_Data = newBlock;
			m_Capacity = newCapacity;
			m_Size = elementsToMove;
		}

		void CheckAndAllocate() noexcept
		{
			if (m_Size >= m_Capacity) Allocate(m_Capacity <= 0 ? 20 : m_Capacity * ARR_RESIZE_MULTIPLIER);
		}

	private:
		void PPushBack(const ValueType& value) noexcept
		{
			CheckAndAllocate();
			Memory::Construct(&m_Data[m_Size], value);
			m_Size++;
		}

		void PPushBack(ValueType&& value) noexcept
		{
			CheckAndAllocate();
			Memory::Construct(&m_Data[m_Size], Move(value));
			m_Size++;
		}

		template <typename... ArgTypes>
		RefType PEmplaceBack(ArgTypes&&... args) noexcept
		{
			CheckAndAllocate();

			PtrType lastLoc = &m_Data[m_Size];

			Memory::Construct(lastLoc, Forward<ArgTypes>(args)...);
			m_Size++;

			ValueType& result = *lastLoc;

			return result;
		}

		template <typename... ArgTypes>
		Iterator PEmplace(ConstIterator it, ArgTypes&&... args) noexcept
		{
			usize offset = it - m_Data;

			PtrType itPtr = m_Data + offset;
			PtrType lastLoc = &m_Data[m_Size];

			CheckAndAllocate();

			if (itPtr == lastLoc)
			{
				Memory::Construct(lastLoc, Forward<ArgTypes>(args)...);
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
				Memory::Construct(itPtr, Forward<ArgTypes>(args)...);
				m_Size++;
			}

			return Iterator(itPtr);
		}

		void PInsertShift(usize offset, usize count) noexcept
		{
			if (count == 0) return;

			usize new_size = m_Size + count;
			if (new_size > m_Capacity)
				Allocate(Max(m_Capacity * ARR_RESIZE_MULTIPLIER, new_size));
			PtrType targetPtr = m_Data + offset;

			usize old_size = m_Size;

			for (usize i = 0; i < count; ++i)
			{
				Memory::Construct(&m_Data[new_size - 1 - i], Move(m_Data[old_size - 1 - i]));
				Memory::Destroy(&m_Data[old_size - 1 - i]);
			}

			for (usize i = old_size - 1 - count; i >= offset; --i)
			{
				m_Data[i + count] = Move(m_Data[i]);
			}

			m_Size = new_size;
		}

		Iterator PInsert(ConstIterator position, const usize count, const ValueType& value)
		{
			usize currentPos = position - CBegin();
			PInsertShift(currentPos, count);

			for (usize i = 0; i < count; ++i)
				Memory::Construct(&m_Data[currentPos + i], value);

			return Begin() + currentPos;
		}

		template<Concept::InputIteratorType InputIterator>
		Iterator PInsert(ConstIterator position, InputIterator first, InputIterator last) noexcept
		{
			usize currentPos = position - CBegin();
			usize count = last - first;
			PInsertShift(currentPos, count);

			for (InputIterator current = first; current != last; ++current)
				Memory::Construct(&m_Data[currentPos + current - first], *current);

			return Begin() + currentPos;
		}

		Iterator PInsert(ConstIterator position, const Utility::InitializerList<ValueType>& list)
		{
			usize currentPos = position - CBegin();
			PInsertShift(currentPos, list.size());

			for (auto current : list)
				Memory::Construct(&m_Data[currentPos + current - list.begin()], *current);

			return Begin() + currentPos;
		}

		void PPopBack() noexcept
		{
			if (m_Size > 0)
			{
				m_Size--;
				Memory::Destroy(&m_Data[m_Size]);
			}
		}

		Iterator PErase(Iterator it) noexcept
		{
			PtrType wherePtr = it.Ptr();

			for (PtrType pos = wherePtr; pos < m_Data + m_Size - 1; ++pos)
				*pos = Move(*(pos + 1));

			Memory::Destroy(m_Data + m_Size - 1);

			--m_Size;

			return Iterator(wherePtr);
		}

		Iterator PErase(ConstIterator it) noexcept
		{
			PtrType wherePtr = it.Ptr();

			for (PtrType pos = wherePtr; pos < m_Data + m_Size - 1; ++pos)
				*pos = Move(*(pos + 1));

			Memory::Destroy(m_Data + m_Size - 1);

			--m_Size;

			return Iterator(wherePtr);
		}

		Iterator PErase(Iterator first, Iterator last) noexcept
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

		Iterator PErase(ConstIterator first, ConstIterator last) noexcept
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

		bool PReserve(usize size) noexcept
		{
			if (size <= m_Capacity)
				return false;

			Allocate(size);

			return true;
		}

		void PShrink() noexcept
		{
			Allocate(m_Size);
			m_Capacity = m_Size;
			m_Data = m_Capacity <= 0 ? nullptr : m_Data;
		}

		void PAppend(const Array& array) noexcept
		{
			usize newSize = m_Size + array.Size();
			if (newSize > m_Capacity) 
				Allocate(Max(m_Capacity * ARR_RESIZE_MULTIPLIER, newSize));
			for (usize i = 0; i < array.Size(); ++i) 
				Memory::Construct(&m_Data[m_Size + i], array.m_Data[i]);
			m_Size = newSize;
		}

		void PAppend(Array&& array) noexcept
		{
			usize newSize = m_Size + array.Size();
			if (newSize > m_Capacity)
				Allocate(Max(m_Capacity * ARR_RESIZE_MULTIPLIER, newSize));
			for (usize i = 0; i < array.Size(); ++i)
				Memory::Construct(&m_Data[m_Size + i], Move(array.m_Data[i]));

			m_Size = newSize;
			array.m_Size = 0;
			array.m_Capacity = 0;
			array.m_Data = nullptr;
		}

	private:
		ValueType* m_Data;
		usize m_Size;
		usize m_Capacity;
	
	private:
		AllocatorType m_Allocator;
	};
}
