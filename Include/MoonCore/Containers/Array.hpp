#pragma once
#include "MoonCore/Containers/Details/ArrayShared.hpp"
#include "MoonCore/Utility/Algorithm.hpp"
#include "MoonCore/Memory/Memory.hpp"
#include "MoonCore/Memory/Allocators/Heap/HeapAllocator.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "MoonCore/Utility/InitializerList.hpp"

/**
 * @file Array.hpp
 * @brief Dynamic contiguous array container.
 * @ingroup seq_containers
 */

namespace MC
{
	/**
	 * @class Array
	 * @brief Dynamically resizable array with contiguous memory storage and custom allocator support.
	 *
	 * `Array<T, Allocator>` is a high-performance sequence container encapsulating a dynamically
	 * allocated array. Elements are stored contiguously in memory, enabling constant-time random access,
	 * cache-friendly iteration, and amortized O(1) insertions at the end.
	 *
	 * Memory is managed via the specified @p Allocator policy, which defaults to `Memory::HeapAllocator<T>`.
	 * When the array grows beyond its capacity, the storage is reallocated geometrically by a factor of 2.
	 *
	 * @tparam T The element type stored in the array. Must satisfy `IsNothrowDestructible<T>`.
	 * @tparam Allocator The memory allocator type satisfying `Concept::HeapAllocatorType`. Defaults to `Memory::HeapAllocator<T>`.
	 *
	 * @code
	 * MC::Array<int> values;
	 * values.Reserve(16);
	 * values.PushBack(10);
	 * values.PushBack(20);
	 * values.EmplaceBack(30);
	 *
	 * for (int val : values) {
	 *     // Process val
	 * }
	 * @endcode
	 */
	template<typename T, 
        Concept::HeapAllocatorType Allocator = Memory::HeapAllocator<T>>
	    requires IsNothrowDestructible<T>
    class Array
	{
	public:
		using ValueType		= T;                       ///< Element type.
		using PtrType		= ValueType*;              ///< Pointer to element.
		using ConstPtrType	= const ValueType*;        ///< Pointer to constant element.
		using RefType		= ValueType&;              ///< Reference to element.
		using ConstRefType	= const ValueType&;        ///< Reference to constant element.

		using AllocatorType = Allocator;               ///< Allocator policy type.

		using Iterator				= ArrayIterator<Array, false>;                ///< Random access iterator.
		using ConstIterator			= ArrayIterator<Array, true>;                 ///< Constant random access iterator.
		using ReverseIterator		= MC::Iterator::ReverseIterator<Iterator>;     ///< Reverse random access iterator.
		using ConstReverseIterator	= MC::Iterator::ReverseIterator<ConstIterator>;///< Constant reverse random access iterator.

	private:
	    static constexpr u8 ARR_RESIZE_MULTIPLIER = 2; ///< Growth factor when capacity is exceeded.

	public:
		/**
		 * @brief Constructs an empty array with an initial capacity of 20 elements.
		 * @note Initial capacity buffer is allocated immediately.
		 */
		Array() noexcept
			: m_Data(nullptr), m_Size(0), m_Capacity(20), m_Allocator(AllocatorType())
		{
			Allocate(m_Capacity);
		}

		/**
		 * @brief Constructs an array with the specified initial size and default-constructs each element.
		 * @param size Number of elements to allocate and construct.
		 */
		Array(usize size) noexcept
			: m_Data(nullptr), m_Size(size), m_Capacity(size), m_Allocator(AllocatorType())
		{
			Allocate(m_Capacity);
			m_Size = size;
			for (usize i = 0; i < m_Size; i++)
				Memory::Construct(&m_Data[i]);
		}

		/**
		 * @brief Copy constructor. Creates a deep copy of elements from @p other.
		 * @param other Source array to copy from.
		 */
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

		/**
		 * @brief Constructs an array populated from an initializer list.
		 * @param other Initializer list containing source elements.
		 */
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

		/**
		 * @brief Move constructor. Takes ownership of @p other's storage without copying.
		 * @param other Source array to move from. Leaves @p other in a valid empty state.
		 */
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

		/**
		 * @brief Destructor. Destroys all elements and deallocates backing buffer.
		 */
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
		/**
		 * @brief Returns the number of elements currently stored.
		 * @return Element count.
		 */
		usize Size() const noexcept { return m_Size; }

		/**
		 * @brief Returns the total number of elements that can be held without reallocation.
		 * @return Allocated capacity.
		 */
		usize Capacity() const noexcept { return m_Capacity; }

		/**
		 * @brief Returns a const pointer to the underlying contiguous storage.
		 * @return Const pointer to buffer.
		 */
		ConstPtrType Data() const noexcept { return m_Data; }

		/**
		 * @brief Returns a mutable pointer to the underlying contiguous storage.
		 * @return Mutable pointer to buffer.
		 */
		PtrType Data() noexcept { return m_Data; }

		/**
		 * @brief Checks whether the container has no elements.
		 * @return True if empty (`Size() == 0`), false otherwise.
		 */
		bool Empty() const noexcept { return m_Size <= 0; }

		/**
		 * @brief Accesses the first element.
		 * @return Reference to first element.
		 * @note Asserts non-empty container in debug builds.
		 */
		RefType Front() noexcept { return *m_Data; }

		/**
		 * @brief Accesses the first element (const overload).
		 * @return Const reference to first element.
		 * @note Asserts non-empty container in debug builds.
		 */
		ConstRefType Front() const noexcept { return *m_Data; }

		/**
		 * @brief Accesses the last element.
		 * @return Reference to last element.
		 * @note Asserts non-empty container in debug builds.
		 */
		RefType Back() noexcept { return *(m_Data + m_Size - 1); }

		/**
		 * @brief Accesses the last element (const overload).
		 * @return Const reference to last element.
		 * @note Asserts non-empty container in debug builds.
		 */
		ConstRefType Back() const noexcept { return *(m_Data + m_Size - 1); }

	public:
		/**
		 * @brief Returns an iterator to the beginning.
		 * @return Mutable iterator to first element.
		 */
		Iterator Begin() noexcept { return Iterator(m_Data); }

		/**
		 * @brief Returns a const iterator to the beginning.
		 * @return Const iterator to first element.
		 */
		ConstIterator Begin() const noexcept { return ConstIterator(m_Data); }

		/**
		 * @brief Returns a const iterator to the beginning.
		 * @return Const iterator to first element.
		 */
		ConstIterator CBegin() const noexcept { return ConstIterator(m_Data); }

		/**
		 * @brief Returns an iterator to the end (one past the last element).
		 * @return Mutable iterator to end.
		 */
		Iterator End() noexcept { return Iterator(m_Data + m_Size); }

		/**
		 * @brief Returns a const iterator to the end.
		 * @return Const iterator to end.
		 */
		ConstIterator End() const noexcept { return ConstIterator(m_Data + m_Size); }

		/**
		 * @brief Returns a const iterator to the end.
		 * @return Const iterator to end.
		 */
		ConstIterator CEnd() const noexcept { return ConstIterator(m_Data + m_Size); }

		/**
		 * @brief Returns a reverse iterator to the beginning of reversed sequence.
		 * @return Reverse iterator pointing to last element.
		 */
		ReverseIterator RBegin() noexcept { return m_Size > 0 ? ReverseIterator(m_Data + m_Size - 1) : REnd(); }

		/**
		 * @brief Returns a const reverse iterator to the beginning of reversed sequence.
		 * @return Const reverse iterator pointing to last element.
		 */
		ConstReverseIterator RBegin() const noexcept { return m_Size > 0 ? ConstReverseIterator(m_Data + m_Size - 1) : REnd(); }

		/**
		 * @brief Returns a const reverse iterator to the beginning of reversed sequence.
		 * @return Const reverse iterator pointing to last element.
		 */
		ConstReverseIterator CRBegin() const noexcept { return m_Size > 0 ? ConstReverseIterator(m_Data + m_Size - 1) : CREnd(); }

		/**
		 * @brief Returns a reverse iterator to the end of reversed sequence.
		 * @return Reverse iterator pointing before first element.
		 */
		ReverseIterator REnd() noexcept { return ReverseIterator(m_Data - 1); }

		/**
		 * @brief Returns a const reverse iterator to the end of reversed sequence.
		 * @return Const reverse iterator pointing before first element.
		 */
		ConstReverseIterator REnd() const noexcept { return ConstReverseIterator(m_Data - 1); }

		/**
		 * @brief Returns a const reverse iterator to the end of reversed sequence.
		 * @return Const reverse iterator pointing before first element.
		 */
		ConstReverseIterator CREnd() const noexcept { return ConstReverseIterator(m_Data - 1); }

	public:
		/** @brief Range-for compatible begin iterator. */
		Iterator begin() noexcept { return Begin(); }
		/** @brief Range-for compatible const begin iterator. */
		ConstIterator begin() const noexcept { return Begin(); }
		/** @brief Range-for compatible const begin iterator. */
		ConstIterator cbegin() const noexcept { return CBegin(); }
		/** @brief Range-for compatible end iterator. */
		Iterator end() noexcept { return End(); }
		/** @brief Range-for compatible const end iterator. */
		ConstIterator end() const noexcept { return End(); }
		/** @brief Range-for compatible const end iterator. */
		ConstIterator cend() const noexcept { return CEnd(); }

		/** @brief Range-for compatible reverse begin iterator. */
		ReverseIterator rbegin() noexcept { return RBegin(); }
		/** @brief Range-for compatible const reverse begin iterator. */
		ConstReverseIterator rbegin() const noexcept { return RBegin(); }
		/** @brief Range-for compatible const reverse begin iterator. */
		ConstReverseIterator crbegin() const noexcept { return CRBegin(); }
		/** @brief Range-for compatible reverse end iterator. */
		ReverseIterator rend() noexcept { return REnd(); }
		/** @brief Range-for compatible const reverse end iterator. */
		ConstReverseIterator rend() const noexcept { return REnd(); }
		/** @brief Range-for compatible const reverse end iterator. */
		ConstReverseIterator crend() const noexcept { return CREnd(); }

	public:
		/**
		 * @brief Copy assignment operator. Replaces contents with deep copy of @p other.
		 * @param other Source array.
		 * @return Reference to this array.
		 */
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

		/**
		 * @brief Move assignment operator. Replaces contents by transferring ownership from @p other.
		 * @param other Source array. Leaves @p other empty.
		 * @return Reference to this array.
		 */
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

		/**
		 * @brief Subscript access operator.
		 * @param index 0-based index of element.
		 * @return Mutable reference to element at index.
		 * @note Asserts `index < Size()` in debug builds.
		 */
		RefType operator[](const usize index) noexcept
		{
			MOON_CHECK(index < m_Size, "Index in array is out of range!");
			return m_Data[index];
		}

		/**
		 * @brief Subscript access operator (const overload).
		 * @param index 0-based index of element.
		 * @return Const reference to element at index.
		 * @note Asserts `index < Size()` in debug builds.
		 */
		ConstRefType operator[](const usize index) const noexcept
		{
			MOON_CHECK(index < m_Size, "Index in array is out of range!");
			return m_Data[index];
		}

	public:
		/**
		 * @brief Destroys all elements, setting size to 0. Allocated capacity remains unchanged.
		 */
		void Clear() noexcept
		{
			if constexpr (!IsTriviallyDestructible<T>)
				for (usize i = 0; i < m_Size; ++i) Memory::Destroy(&m_Data[i]);
			m_Size = 0;
		}

		/**
		 * @brief Appends all elements from another array (copy).
		 * @param other Source array to append.
		 */
		void Append(const Array& other) noexcept { PAppend(other); }

		/**
		 * @brief Appends all elements from another array (move).
		 * @param other Source array to append and drain.
		 */
		void Append(Array&& other) noexcept { PAppend(Move(other)); }

		/**
		 * @brief Shrinks allocated buffer to fit the current size exactly.
		 */
		void Shrink() noexcept { PShrink(); }

		/**
		 * @brief Ensures that capacity is at least @p size elements.
		 * @param size Desired minimum capacity.
		 * @return True if reallocation occurred, false if capacity was already sufficient.
		 */
		bool Reserve(usize size) noexcept { return PReserve(size); }

		/**
		 * @brief Resizes the array to contain @p size elements, filling new elements with @p fill.
		 * @param size Target size.
		 * @param fill Value to copy into newly created elements.
		 */
		void Resize(usize size, const ValueType& fill) noexcept
		{
			static_assert(IsCopyConstructible<ValueType>, "Resize requires T to be CopyConstructible!");

			if (size < m_Size && m_Size < m_Capacity) { m_Size = size; return; }
			if (size >= m_Capacity) Allocate(size + 1);

			for (ValueType* current = m_Data + m_Size; current != m_Data + size; ++current)
				Memory::Construct(current, fill);
			m_Size = size;
		}

		/**
		 * @brief Resizes the array to contain @p size elements, constructing new elements in-place with @p args.
		 * @tparam ArgTypes Constructor argument types for T.
		 * @param size Target size.
		 * @param args Arguments forwarded to constructor of T.
		 */
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
		/**
		 * @brief Inserts a copy of @p value before @p position.
		 * @param position Iterator before which the element will be inserted.
		 * @param value Element to copy.
		 * @return Iterator pointing to the inserted element.
		 */
		Iterator Insert(ConstIterator position, const ValueType& value) noexcept { return PEmplace(position, value); }

		/**
		 * @brief Inserts @p value before @p position by moving.
		 * @param position Iterator before which the element will be inserted.
		 * @param value Element to move.
		 * @return Iterator pointing to the inserted element.
		 */
		Iterator Insert(ConstIterator position, ValueType&& value) noexcept { return PEmplace(position, Move(value)); }

		/**
		 * @brief Inserts @p count copies of @p value before @p position.
		 * @param position Iterator before which the elements will be inserted.
		 * @param count Number of elements to insert.
		 * @param value Element value to replicate.
		 * @return Iterator pointing to the first inserted element.
		 */
		Iterator Insert(ConstIterator position, const usize count, const ValueType& value) noexcept { return PInsert(position, count, value); }

		/**
		 * @brief Inserts elements from an initializer list before @p position.
		 * @param position Iterator before which the elements will be inserted.
		 * @param list Initializer list of elements.
		 * @return Iterator pointing to the first inserted element.
		 */
		Iterator Insert(ConstIterator position, const Utility::InitializerList<ValueType>& list) noexcept { return PInsert(position, list); }

		/**
		 * @brief Inserts elements from the range `[first, last)` before @p position.
		 * @tparam InputIterator Iterator type satisfying `Concept::InputIteratorType`.
		 * @param position Iterator before which the elements will be inserted.
		 * @param first Beginning of input range.
		 * @param last End of input range.
		 * @return Iterator pointing to the first inserted element.
		 */
		template<Concept::InputIteratorType InputIterator> Iterator Insert(ConstIterator position, InputIterator first, InputIterator last) noexcept { return PInsert<InputIterator>(position, first, last); }

		/**
		 * @brief Constructs an element in-place before @p it.
		 * @tparam ArgTypes Argument types forwarded to constructor of T.
		 * @param it Iterator before which the element will be constructed.
		 * @param args Arguments forwarded to constructor.
		 * @return Iterator pointing to the emplaced element.
		 */
		template<typename... ArgTypes> Iterator Emplace(ConstIterator it, ArgTypes&&... args) noexcept { return PEmplace(it, Forward<ArgTypes>(args)...); }

		/**
		 * @brief Appends a copy of @p value to the end of the array.
		 * @param value Element value to copy.
		 * @note Amortized O(1) time complexity.
		 */
		void PushBack(const ValueType& value) noexcept { PPushBack(value); }

		/**
		 * @brief Appends @p value to the end of the array by moving.
		 * @param value Element value to move.
		 * @note Amortized O(1) time complexity.
		 */
		void PushBack(ValueType&& value) noexcept { PPushBack(Move(value)); }

		/**
		 * @brief Constructs an element in-place at the end of the array.
		 * @tparam ArgTypes Argument types forwarded to constructor of T.
		 * @param args Arguments forwarded to constructor.
		 * @return Reference to the newly constructed element.
		 * @note Amortized O(1) time complexity.
		 */
		template<typename... ArgTypes> RefType EmplaceBack(ArgTypes&&... args) noexcept { return PEmplaceBack(Forward<ArgTypes>(args)...); }

		/**
		 * @brief Erases the element pointed to by @p it.
		 * @param it Iterator pointing to element to remove.
		 * @return Iterator following the removed element.
		 */
		Iterator Erase(Iterator it) noexcept { return PErase(it); }

		/**
		 * @brief Erases the element pointed to by @p it (const iterator overload).
		 * @param it Const iterator pointing to element to remove.
		 * @return Iterator following the removed element.
		 */
		Iterator Erase(ConstIterator it) noexcept { return PErase(it); }

		/**
		 * @brief Erases elements in the range `[first, last)`.
		 * @param first Beginning of range to remove.
		 * @param last End of range to remove.
		 * @return Iterator following the last removed element.
		 */
		Iterator Erase(Iterator first, Iterator last) noexcept { return PErase(first, last); }

		/**
		 * @brief Erases elements in the range `[first, last)` (const iterator overload).
		 * @param first Beginning of range to remove.
		 * @param last End of range to remove.
		 * @return Iterator following the last removed element.
		 */
		Iterator Erase(ConstIterator first, ConstIterator last) noexcept { return PErase(first, last); }

		/**
		 * @brief Erases the element at the specified @p index.
		 * @param index 0-based position to erase.
		 * @return Iterator following the removed element.
		 */
		Iterator EraseAt(usize index) noexcept { return PErase(Begin() + index); }

		/**
		 * @brief Removes the last element from the array.
		 * @note Does nothing if array is empty. O(1) time complexity.
		 */
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
		ValueType* m_Data;       ///< Backing raw storage.
		usize m_Size;            ///< Number of currently active elements.
		usize m_Capacity;        ///< Allocated capacity of backing buffer.
	
	private:
		AllocatorType m_Allocator; ///< Allocator policy instance.
	};
}
