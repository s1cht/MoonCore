#pragma once
#include "MoonCore/Containers/Details/ArrayShared.hpp"
#include "MoonCore/Memory/Memory.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "MoonCore/Utility/InitializerList.hpp"
#include "MoonCore/Utility/Nullable.hpp"
#include "MoonCore/Utility/Algorithm.hpp"

/**
 * @file StackArray.hpp
 * @brief Dynamic-size array with compile-time fixed maximum capacity stored on the stack.
 * @ingroup seq_containers
 */

namespace MC
{
	/**
	 * @class StackArrayImpl
	 * @brief Dynamically sized array with fixed maximum capacity allocated entirely on the stack.
	 *
	 * `StackArrayImpl<T, ArraySize>` provides a vector-like sequence container with runtime size
	 * tracking up to a compile-time upper bound (@p ArraySize). Elements are stored in-place inside an uninitialized
	 * union buffer, avoiding heap allocations while only constructing elements that are actively in use.
	 *
	 * Provides three categories of insertion operations:
	 * 1. **Checked**: Asserts capacity availability in debug mode (`PushBack`, `EmplaceBack`, `Insert`, `Emplace`).
	 * 2. **Fallible (Try)**: Returns boolean / nullable failure on overflow (`TryPushBack`, `TryEmplaceBack`, `TryInsert`, `TryEmplace`).
	 * 3. **Unchecked**: Maximum performance without bounds checks when capacity is pre-validated (`UncheckedPushBack`, etc.).
	 *
	 * @tparam T The element type stored in the array. Must satisfy `IsNothrowDestructible<T>`.
	 * @tparam ArraySize Maximum number of elements that can be stored on the stack.
	 *
	 * @code
	 * MC::StackArray<int, 16> localBuffer;
	 * localBuffer.PushBack(1);
	 * localBuffer.PushBack(2);
	 * localBuffer.EmplaceBack(3);
	 *
	 * for (int x : localBuffer) {
	 *     // Process element
	 * }
	 * @endcode
	 */
	template<typename T, usize ArraySize>
		requires IsNothrowDestructible<T>
	class StackArrayImpl
	{
	public:
		using ValueType = T;                                           ///< Stored element type.
		using PtrType = ValueType*;                                    ///< Pointer to element.
		using ConstPtrType = const ValueType*;                          ///< Pointer to const element.
		using RefType = ValueType&;                                    ///< Reference to element.
		using ConstRefType = const ValueType&;                          ///< Reference to const element.

		using Iterator = ArrayIterator<StackArrayImpl, false>;          ///< Random-access iterator.
		using ConstIterator = ArrayIterator<StackArrayImpl, true>;      ///< Const random-access iterator.
		using ReverseIterator = MC::Iterator::ReverseIterator<Iterator>;///< Reverse random-access iterator.
		using ConstReverseIterator = MC::Iterator::ReverseIterator<ConstIterator>; ///< Const reverse iterator.

	public:
		/**
		 * @brief Constructs an empty array with size 0.
		 */
		constexpr StackArrayImpl() noexcept
			: m_Size(0)
		{}

		/**
		 * @brief Constructs an array with @p size default-constructed elements.
		 * @param size Number of elements to construct. Must be <= @p ArraySize.
		 * @note Asserts `size <= ArraySize` in debug builds.
		 */
		constexpr StackArrayImpl(usize size) noexcept
			: m_Size(size)
		{
			MOON_CHECK(size <= ArraySize, "Given size > than ArraySize");
			m_Size = size;
			for (usize i = 0; i < m_Size; i++)
				Memory::Construct(&m_Data[i]);
		}

		/**
		 * @brief Constructs an array from an initializer list.
		 * @param other Initializer list with count <= @p ArraySize.
		 */
		constexpr StackArrayImpl(const Utility::InitializerList<ValueType>& other) noexcept
			: m_Size(other.size())
		{
			MOON_CHECK(m_Size <= ArraySize, "initializer_list overflows StackArrayImpl");
			for (usize i = 0; i < other.size(); i++)
				Memory::Construct(&m_Data[i], other.begin()[i]);
		}

		/**
		 * @brief Copy constructor. Copies active elements from @p other.
		 * @param other Source array.
		 */
		constexpr StackArrayImpl(const StackArrayImpl& other) noexcept
			: m_Size(other.m_Size)
		{
			for (usize i = 0; i < m_Size; i++)
				Memory::Construct(&m_Data[i], other.m_Data[i]);
		}

		/**
		 * @brief Move constructor. Moves active elements from @p other and clears @p other.
		 * @param other Source array to move from.
		 */
		constexpr StackArrayImpl(StackArrayImpl&& other) noexcept
		{
			m_Size = other.m_Size;
			other.m_Size = 0;

			for (usize i = 0; i < m_Size; i++)
				Memory::Construct(&m_Data[i], Move(other.m_Data[i]));
		}

		/**
		 * @brief Destructor. Destroys all active elements.
		 */
		constexpr ~StackArrayImpl() noexcept
		{
			Clear();
		}

	public:
		/**
		 * @brief Returns the number of active elements.
		 * @return Current element count.
		 */
		constexpr usize Size() const noexcept { return m_Size; }

		/**
		 * @brief Returns the fixed maximum capacity of the stack buffer.
		 * @return Maximum capacity (`ArraySize`).
		 */
		static constexpr usize Capacity() noexcept { return ArraySize; }

		/**
		 * @brief Returns a const pointer to the underlying buffer.
		 * @return Const pointer.
		 */
		constexpr ConstPtrType Data() const noexcept { return m_Data; }

		/**
		 * @brief Returns a mutable pointer to the underlying buffer.
		 * @return Mutable pointer.
		 */
		constexpr PtrType Data() noexcept { return m_Data; }

		/**
		 * @brief Checks if the array contains no elements.
		 * @return True if `Size() == 0`, false otherwise.
		 */
		constexpr bool Empty() const noexcept { return m_Size <= 0; }

		/**
		 * @brief Accesses the first element.
		 * @return Mutable reference to first element.
		 */
		constexpr RefType Front() noexcept { return *m_Data; }

		/**
		 * @brief Accesses the first element (const overload).
		 * @return Const reference to first element.
		 */
		constexpr ConstRefType Front() const noexcept { return *m_Data; }

		/**
		 * @brief Accesses the last element.
		 * @return Mutable reference to last element.
		 */
		constexpr RefType Back() noexcept { return *(m_Data + m_Size - 1); }

		/**
		 * @brief Accesses the last element (const overload).
		 * @return Const reference to last element.
		 */
		constexpr ConstRefType Back() const noexcept { return *(m_Data + m_Size - 1); }

	public:
		/** @brief Returns an iterator to the beginning. */
		constexpr Iterator Begin() noexcept { return Iterator(m_Data); }
		/** @brief Returns a const iterator to the beginning. */
		constexpr ConstIterator CBegin() const noexcept { return ConstIterator(m_Data); }
		/** @brief Returns an iterator to the end. */
		constexpr Iterator End() noexcept { return Iterator(m_Data + m_Size); }
		/** @brief Returns a const iterator to the end. */
		constexpr ConstIterator CEnd() const noexcept { return ConstIterator(m_Data + m_Size); }

		/** @brief Returns a reverse iterator to the beginning of reversed sequence. */
		constexpr ReverseIterator RBegin() noexcept { return m_Size > 0 ? ReverseIterator(m_Data + m_Size - 1) : REnd(); }
		/** @brief Returns a const reverse iterator to the beginning of reversed sequence. */
		constexpr ConstReverseIterator CRBegin() const noexcept { return m_Size > 0 ? ConstReverseIterator(m_Data + m_Size - 1) : CREnd(); }
		/** @brief Returns a reverse iterator to the end of reversed sequence. */
		constexpr ReverseIterator REnd() noexcept { return ReverseIterator(m_Data - 1); }
		/** @brief Returns a const reverse iterator to the end of reversed sequence. */
		constexpr ConstReverseIterator CREnd() const noexcept { return ConstReverseIterator(m_Data - 1); }

	public:
		/** @brief Range-for compatible begin iterator. */
		constexpr Iterator begin() noexcept { return Begin(); }
		/** @brief Range-for compatible const begin iterator. */
		constexpr ConstIterator cbegin() const noexcept { return CBegin(); }
		/** @brief Range-for compatible end iterator. */
		constexpr Iterator end() noexcept { return End(); }
		/** @brief Range-for compatible const end iterator. */
		constexpr ConstIterator cend() const noexcept { return CEnd(); }

		/** @brief Range-for compatible reverse begin iterator. */
		constexpr ReverseIterator rbegin() noexcept { return RBegin(); }
		/** @brief Range-for compatible const reverse begin iterator. */
		constexpr ConstReverseIterator crbegin() const noexcept { return CRBegin(); }
		/** @brief Range-for compatible reverse end iterator. */
		constexpr ReverseIterator rend() noexcept { return REnd(); }
		/** @brief Range-for compatible const reverse end iterator. */
		constexpr ConstReverseIterator crend() const noexcept { return CREnd(); }

	public:
		/**
		 * @brief Copy assignment operator. Replaces contents with deep copy of @p other.
		 * @param other Source array.
		 * @return Reference to this array.
		 */
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

		/**
		 * @brief Move assignment operator. Transfers elements from @p other and clears @p other.
		 * @param other Source array.
		 * @return Reference to this array.
		 */
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

		/**
		 * @brief Subscript access operator.
		 * @param index 0-based index.
		 * @return Mutable reference to element.
		 * @note Asserts `index < Size()` in debug builds.
		 */
		constexpr RefType operator[](const usize index) noexcept
		{
			MOON_CHECK(index < m_Size, "Index in array is out of range!");
			return m_Data[index];
		}

		/**
		 * @brief Subscript access operator (const overload).
		 * @param index 0-based index.
		 * @return Const reference to element.
		 * @note Asserts `index < Size()` in debug builds.
		 */
		constexpr ConstRefType operator[](const usize index) const noexcept
		{
			MOON_CHECK(index < m_Size, "Index in array is out of range!");
			return m_Data[index];
		}

		/**
		 * @brief Subscript access operator with scoped enum index.
		 * @tparam Enum Scoped enum type satisfying `Concept::ScopedEnum`.
		 * @param index Enum value cast to index.
		 * @return Mutable reference to element.
		 */
		template<Concept::ScopedEnum Enum>
		constexpr RefType operator[](const Enum index) noexcept
		{
			MOON_CHECK(static_cast<usize>(index) < m_Size, "Index in array is out of range!");
			return m_Data[static_cast<usize>(index)];
		}

		/**
		 * @brief Subscript access operator with scoped enum index (const overload).
		 * @tparam Enum Scoped enum type satisfying `Concept::ScopedEnum`.
		 * @param index Enum value cast to index.
		 * @return Const reference to element.
		 */
		template<Concept::ScopedEnum Enum>
		constexpr ConstRefType operator[](const Enum index) const noexcept
		{
			MOON_CHECK(static_cast<usize>(index) < m_Size, "Index in array is out of range!");
			return m_Data[static_cast<usize>(index)];
		}

	public:
		/**
		 * @brief Destroys all active elements and resets size to 0.
		 */
		constexpr void Clear() noexcept
		{
			if constexpr (!IsTriviallyDestructible<T>)
				for (usize i = 0; i < m_Size; ++i) Memory::Destroy(&m_Data[i]);
			m_Size = 0;
		}

		/**
		 * @brief Resizes array to @p size default-constructed elements.
		 * @param size Target size. Must not exceed @p ArraySize.
		 */
		constexpr void Resize(usize size) noexcept
		{
			static_assert(IsMoveConstructible<ValueType>, "Resize requires T to be MoveConstructible!");
			static_assert(IsDefaultConstructible<ValueType>, "Resize requires T to be DefaultConstructible!");

			Resize(size, ValueType());
		}

		/**
		 * @brief Resizes array to @p size elements, filling newly created elements with copies of @p fill.
		 * @param size Target size. Must not exceed @p ArraySize.
		 * @param fill Value to copy into new elements.
		 */
		constexpr void Resize(usize size, const ValueType& fill) noexcept
		{
			static_assert(IsCopyConstructible<ValueType>, "Resize requires T to be CopyInsertable!");
			MOON_CHECK(size <= ArraySize, "Size > ArraySize");

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
		/**
		 * @brief Inserts a copy of @p value before @p position.
		 * @param position Iterator before which element is inserted.
		 * @param value Element value to copy.
		 * @return Iterator pointing to inserted element.
		 * @note Asserts capacity is available in debug builds.
		 */
		constexpr Iterator Insert(ConstIterator position, const ValueType& value) noexcept
		{
			MOON_CHECK(m_Size < ArraySize, "StackArrayImpl overflow detected");
			return UncheckedEmplaceImpl(position, value);
		}

		/**
		 * @brief Inserts @p value before @p position by moving.
		 * @param position Iterator before which element is inserted.
		 * @param value Element value to move.
		 * @return Iterator pointing to inserted element.
		 * @note Asserts capacity is available in debug builds.
		 */
		constexpr Iterator Insert(ConstIterator position, ValueType&& value) noexcept
		{
			MOON_CHECK(m_Size < ArraySize, "StackArrayImpl overflow detected");
			return UncheckedEmplaceImpl(position, Move(value));
		}

		/**
		 * @brief Inserts @p count copies of @p value before @p position.
		 * @param position Iterator before which elements are inserted.
		 * @param count Number of elements to insert.
		 * @param value Value to replicate.
		 * @return Iterator pointing to first inserted element.
		 */
		constexpr Iterator Insert(ConstIterator position, const usize count, const ValueType& value) noexcept
		{
			MOON_CHECK(m_Size + count <= ArraySize, "StackArrayImpl overflow detected");
			return UncheckedInsertImpl(position, count, value);
		}

		/**
		 * @brief Inserts elements from an initializer list before @p position.
		 * @param position Iterator before which elements are inserted.
		 * @param list Initializer list.
		 * @return Iterator pointing to first inserted element.
		 */
		constexpr Iterator Insert(ConstIterator position, const Utility::InitializerList<ValueType>& list) noexcept
		{
			MOON_CHECK(m_Size + list.size() <= ArraySize, "StackArrayImpl overflow detected");
			return UncheckedInsertImpl(position, list);
		}

		/**
		 * @brief Inserts elements from the range `[first, last)` before @p position.
		 * @tparam InputIterator Input iterator type.
		 * @param position Iterator before which elements are inserted.
		 * @param first Beginning of range.
		 * @param last End of range.
		 * @return Iterator pointing to first inserted element.
		 */
		template<Concept::InputIteratorType InputIterator> constexpr Iterator Insert(ConstIterator position, InputIterator first, InputIterator last) noexcept
		{
			MOON_CHECK(m_Size + last - first <= ArraySize, "StackArrayImpl overflow detected");
			return UncheckedInsertImpl<InputIterator>(position, first, last);
		}

		/**
		 * @brief Attempts to insert a copy of @p value before @p position without asserting on overflow.
		 * @param position Iterator before which element is inserted.
		 * @param value Element to copy.
		 * @return Iterator to inserted element, or `End()` if capacity was exceeded.
		 */
		constexpr Iterator TryInsert(ConstIterator position, const ValueType& value) noexcept
		{
			if (m_Size >= ArraySize) return End();
			return UncheckedEmplaceImpl(position, value);
		}

		/**
		 * @brief Attempts to insert @p value by moving before @p position without asserting on overflow.
		 * @param position Iterator before which element is inserted.
		 * @param value Element to move.
		 * @return Iterator to inserted element, or `End()` if capacity was exceeded.
		 */
		constexpr Iterator TryInsert(ConstIterator position, ValueType&& value) noexcept
		{
			if (m_Size >= ArraySize) return End();
			return UncheckedEmplaceImpl(position, Move(value));
		}

		/**
		 * @brief Attempts to insert @p count copies of @p value before @p position.
		 * @param position Iterator before which elements are inserted.
		 * @param count Number of elements to insert.
		 * @param value Element value.
		 * @return Iterator to first inserted element, or `End()` on overflow.
		 */
		constexpr Iterator TryInsert(ConstIterator position, const usize count, const ValueType& value) noexcept
		{
			if (m_Size + count > ArraySize) return End();
			return UncheckedInsertImpl(position, count, value);
		}

		/**
		 * @brief Attempts to insert elements from an initializer list before @p position.
		 * @param position Iterator before which elements are inserted.
		 * @param list Initializer list.
		 * @return Iterator to first inserted element, or `End()` on overflow.
		 */
		constexpr Iterator TryInsert(ConstIterator position, const Utility::InitializerList<ValueType>& list) noexcept
		{
			if (m_Size + list.size() > ArraySize) return End();
			return UncheckedInsertImpl(position, list);
		}

		/**
		 * @brief Attempts to insert elements from input range `[first, last)`.
		 * @tparam InputIterator Input iterator type.
		 * @param position Iterator before which elements are inserted.
		 * @param first Beginning of range.
		 * @param last End of range.
		 * @return Iterator to first inserted element, or `End()` on overflow.
		 */
		template<Concept::InputIteratorType InputIterator> constexpr Iterator TryInsert(ConstIterator position, InputIterator first, InputIterator last) noexcept
		{
			if (m_Size + last - first > ArraySize) return End();
			return UncheckedInsertImpl<InputIterator>(position, first, last);
		}

		/** @brief Unchecked insertion of copy of @p value. */
		constexpr Iterator UncheckedInsert(ConstIterator position, const ValueType& value) noexcept
		{
			return UncheckedEmplaceImpl(position, value);
		}
		/** @brief Unchecked insertion of moved @p value. */
		constexpr Iterator UncheckedInsert(ConstIterator position, ValueType&& value) noexcept
		{
			return UncheckedEmplaceImpl(position, Move(value));
		}
		/** @brief Unchecked insertion of @p count copies of @p value. */
		constexpr Iterator UncheckedInsert(ConstIterator position, const usize count, const ValueType& value) noexcept
		{
			return UncheckedInsertImpl(position, count, value);
		}
		/** @brief Unchecked insertion of initializer list. */
		constexpr Iterator UncheckedInsert(ConstIterator position, const Utility::InitializerList<ValueType>& list) noexcept
		{
			return UncheckedInsertImpl(position, list);
		}
		/** @brief Unchecked insertion of input range. */
		template<Concept::InputIteratorType InputIterator> constexpr Iterator UncheckedInsert(ConstIterator position, InputIterator first, InputIterator last) noexcept
		{
			return UncheckedInsertImpl<InputIterator>(position, first, last);
		}

		/**
		 * @brief Constructs an element in-place before @p it.
		 * @tparam ArgTypes Argument types for T's constructor.
		 * @param it Iterator position.
		 * @param args Forwarded constructor arguments.
		 * @return Iterator to constructed element.
		 */
		template<typename... ArgTypes> constexpr Iterator Emplace(ConstIterator it, ArgTypes&&... args) noexcept
		{
			MOON_CHECK(m_Size < ArraySize, "StackArrayImpl overflow detected");
			return UncheckedEmplaceImpl(it, Forward<ArgTypes>(args)...);
		}

		/**
		 * @brief Attempts to emplace an element in-place before @p it without asserting on overflow.
		 * @tparam ArgTypes Argument types for T's constructor.
		 * @param it Iterator position.
		 * @param args Forwarded constructor arguments.
		 * @return Iterator to constructed element, or `End()` on overflow.
		 */
		template<typename... ArgTypes> constexpr Iterator TryEmplace(ConstIterator it, ArgTypes&&... args) noexcept
		{
			if (m_Size >= ArraySize) return End();
			return UncheckedEmplaceImpl(it, Forward<ArgTypes>(args)...);
		}

		/** @brief Unchecked in-place construction before @p it. */
		template<typename... ArgTypes> constexpr Iterator UncheckedEmplace(ConstIterator it, ArgTypes&&... args) noexcept
		{
			return UncheckedEmplaceImpl(it, Forward<ArgTypes>(args)...);
		}

		/**
		 * @brief Appends a copy of @p value to the end.
		 * @param value Element to copy.
		 * @note Asserts capacity is available in debug builds.
		 */
		constexpr void PushBack(const ValueType& value) noexcept
		{
			MOON_CHECK(m_Size < ArraySize, "StackArrayImpl overflow detected");
			UncheckedPushBackImpl(value);
		}

		/**
		 * @brief Appends @p value to the end by moving.
		 * @param value Element to move.
		 * @note Asserts capacity is available in debug builds.
		 */
		constexpr void PushBack(ValueType&& value) noexcept
		{
			MOON_CHECK(m_Size < ArraySize, "StackArrayImpl overflow detected");
			UncheckedPushBackImpl(Move(value));
		}

		/**
		 * @brief Attempts to append a copy of @p value, returning false if capacity is full.
		 * @param value Element to copy.
		 * @return True on success, false if full.
		 */
		constexpr bool TryPushBack(const ValueType& value) noexcept
		{
			if (m_Size >= ArraySize) return false;
			UncheckedPushBackImpl(value);
			return true;
		}

		/**
		 * @brief Attempts to append @p value by moving, returning false if capacity is full.
		 * @param value Element to move.
		 * @return True on success, false if full.
		 */
		constexpr bool TryPushBack(ValueType&& value) noexcept
		{
			if (m_Size >= ArraySize) return false;
			UncheckedPushBackImpl(Move(value));
			return true;
		}

		/** @brief Unchecked append of copy of @p value. */
		constexpr void UncheckedPushBack(const ValueType& value) noexcept
		{
			UncheckedPushBackImpl(value);
		}
		/** @brief Unchecked append of moved @p value. */
		constexpr void UncheckedPushBack(ValueType&& value) noexcept
		{
			UncheckedPushBackImpl(Move(value));
		}

		/**
		 * @brief Constructs an element in-place at the end.
		 * @tparam ArgTypes Argument types for T's constructor.
		 * @param args Forwarded constructor arguments.
		 * @return Reference to newly constructed element.
		 */
		template<typename... ArgTypes> constexpr RefType EmplaceBack(ArgTypes&&... args) noexcept
		{
			MOON_CHECK(m_Size < ArraySize, "StackArrayImpl overflow detected");
			return UncheckedEmplaceBackImpl(Forward<ArgTypes>(args)...);
		}

		/**
		 * @brief Attempts to emplace an element at the end, returning a `Nullable<RefType>`.
		 * @tparam ArgTypes Argument types for T's constructor.
		 * @param args Forwarded constructor arguments.
		 * @return Nullable reference to newly constructed element, or null on overflow.
		 */
		template<typename... ArgTypes> constexpr Utility::Nullable<RefType> TryEmplaceBack(ArgTypes&&... args) noexcept
		{
			if (m_Size >= ArraySize) return Utility::Null;
			return Utility::MakeNullable(UncheckedEmplaceBackImpl(Forward<ArgTypes>(args)...));
		}

		/** @brief Unchecked in-place construction at the end. */
		template<typename... ArgTypes> constexpr RefType UncheckedEmplaceBack(ArgTypes&&... args) noexcept
		{
			return UncheckedEmplaceBackImpl(Forward<ArgTypes>(args)...);
		}

		/** @brief Erases element at iterator position. */
		constexpr Iterator Erase(Iterator it) noexcept { return EraseImpl(it); }
		/** @brief Erases element at const iterator position. */
		constexpr Iterator Erase(ConstIterator it) noexcept { return EraseImpl(it); }
		/** @brief Erases elements in range `[first, last)`. */
		constexpr Iterator Erase(Iterator first, Iterator last) noexcept { return EraseImpl(first, last); }
		/** @brief Erases elements in range `[first, last)`. */
		constexpr Iterator Erase(ConstIterator first, ConstIterator last) noexcept { return EraseImpl(first, last); }

		/**
		 * @brief Erases element at 0-based index.
		 * @param index Element position to erase.
		 * @return Iterator following removed element.
		 */
		constexpr Iterator EraseAt(usize index) noexcept { return EraseImpl(Begin() + index); }

		/**
		 * @brief Removes the last element from the array.
		 */
		constexpr void PopBack() noexcept { PopBackImpl(); }

		/**
		 * @brief No-op shrink provided for API compatibility with `MC::Array`.
		 */
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
		union { ValueType m_Data[ArraySize]; }; ///< In-place uninitialized union storage.
		usize m_Size;                           ///< Current number of constructed elements.
	};

	/**
	 * @brief Convenient alias for `StackArrayImpl`, accepting integer constants or counted scoped enums.
	 * @tparam T Stored element type.
	 * @tparam ArraySize Maximum capacity as an integer or scoped enum with `Count`.
	 */
	template<typename T, auto ArraySize> using StackArray = StackArrayImpl<T, ToArraySize<ArraySize>()>;
}
