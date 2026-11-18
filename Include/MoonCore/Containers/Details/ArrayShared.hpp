#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Utility/Iterator.hpp"

/**
 * @file ArrayShared.hpp
 * @brief Common array iterators and helper metaprogramming utilities.
 * @ingroup seq_containers
 */

namespace MC
{
	/**
	 * @brief Converts a compile-time value or scoped enum to an array size.
	 *
	 * Allows passing either an integer literal or an enum class value to size fixed-size containers.
	 *
	 * @tparam ArraySize Compile-time integral value or counted scoped enum.
	 * @return The size converted to `usize`.
	 */
	template<auto ArraySize>
	    requires Concept::CountedScopedEnum<decltype(ArraySize)> || Concept::Integral<decltype(ArraySize)>
	consteval usize ToArraySize() noexcept
	{
		if constexpr (Concept::Integral<decltype(ArraySize)>)
			return static_cast<usize>(ArraySize);
		else
			return static_cast<usize>(ToUnderlying(ArraySize));
	}

	/**
	 * @class ArrayIterator
	 * @brief Random-access iterator for contiguous array containers.
	 *
	 * Implements standard random-access iterator semantics for contiguous buffer containers
	 * such as `MC::Array`, `MC::FixedArray`, and `MC::StackArray`.
	 *
	 * @tparam Container The owning container type providing `ValueType`.
	 * @tparam IsConst Whether this iterator provides const access to elements.
	 */
	template<typename Container, bool IsConst = false>
	class ArrayIterator
	{
	public:
		using ValueType = typename Container::ValueType;       ///< Element value type.
		using DiffType = iptr;                                 ///< Difference type between iterators.
		using PtrType = Conditional<IsConst, const ValueType*, ValueType*>; ///< Pointer to element.
		using RefType = Conditional<IsConst, const ValueType&, ValueType&>; ///< Reference to element.
		using Category = Iterator::Tags::RandomAccess;         ///< Random-access iterator tag.

	public:
		/**
		 * @brief Default constructs an invalid or null iterator.
		 */
		constexpr ArrayIterator() noexcept : m_Ptr(nullptr) {}

		/**
		 * @brief Constructs an iterator pointing to the given element.
		 * @param ptr Pointer to the element in contiguous memory.
		 */
		constexpr ArrayIterator(PtrType ptr) noexcept : m_Ptr(ptr) {}

	public:
		/**
		 * @brief Advances the iterator forward by @p index positions.
		 * @param index Number of positions to advance.
		 * @return Reference to this iterator.
		 */
		constexpr ArrayIterator& operator+=(usize index) noexcept { m_Ptr += index; return *this; }

		/**
		 * @brief Advances the iterator backward by @p index positions.
		 * @param index Number of positions to move backward.
		 * @return Reference to this iterator.
		 */
		constexpr ArrayIterator& operator-=(usize index) noexcept { m_Ptr -= index; return *this; }

		/**
		 * @brief Computes a new iterator advanced by @p index positions.
		 * @param index Offset from current position.
		 * @return Advanced iterator.
		 */
		constexpr ArrayIterator operator+(usize index) const noexcept { return ArrayIterator(m_Ptr + index); }

		/**
		 * @brief Computes a new iterator moved back by @p index positions.
		 * @param index Offset from current position.
		 * @return Decremented iterator.
		 */
		constexpr ArrayIterator operator-(usize index) const noexcept { return ArrayIterator(m_Ptr - index); }

		/**
		 * @brief Prefix increment operator. Advances to next element.
		 * @return Reference to advanced iterator.
		 */
		constexpr ArrayIterator& operator++() noexcept { ++m_Ptr; return *this; }

		/**
		 * @brief Postfix increment operator. Advances to next element.
		 * @return Copy of iterator before advancement.
		 */
		constexpr ArrayIterator operator++(int) noexcept { ArrayIterator temp = *this; ++m_Ptr; return temp; }

		/**
		 * @brief Prefix decrement operator. Moves to previous element.
		 * @return Reference to decremented iterator.
		 */
		constexpr ArrayIterator& operator--() noexcept { --m_Ptr; return (*this); }

		/**
		 * @brief Postfix decrement operator. Moves to previous element.
		 * @return Copy of iterator before decrement.
		 */
		constexpr ArrayIterator operator--(int) noexcept { ArrayIterator temp = *this; --m_Ptr; return temp; }

		/**
		 * @brief Computes the distance between two iterators.
		 * @param it Iterator to compare with.
		 * @return Signed distance in elements.
		 */
		constexpr DiffType operator-(const ArrayIterator& it) const noexcept { return m_Ptr - it.m_Ptr; }

		/**
		 * @brief Equality comparison between two iterators.
		 * @param it Other iterator.
		 * @return True if pointing to the same address.
		 */
		constexpr bool operator==(const ArrayIterator& it) const noexcept { return m_Ptr == it.m_Ptr; }

		/**
		 * @brief Inequality comparison between two iterators.
		 * @param it Other iterator.
		 * @return True if pointing to different addresses.
		 */
		constexpr bool operator!=(const ArrayIterator& it) const noexcept { return m_Ptr != it.m_Ptr; }

		/**
		 * @brief Less-than ordering comparison.
		 * @param it Other iterator.
		 * @return True if this iterator points to an earlier memory address.
		 */
		constexpr bool operator<(const ArrayIterator& it) const noexcept { return m_Ptr < it.m_Ptr; }

		/**
		 * @brief Greater-than ordering comparison.
		 * @param it Other iterator.
		 * @return True if this iterator points to a later memory address.
		 */
		constexpr bool operator>(const ArrayIterator& it) const noexcept { return m_Ptr > it.m_Ptr; }

		/**
		 * @brief Subscript access relative to current position.
		 * @param index Element offset.
		 * @return Reference to element at offset.
		 */
		constexpr RefType operator[](usize index) const noexcept { return *(m_Ptr + index); }

		/**
		 * @brief Member access operator.
		 * @return Raw pointer to element.
		 */
		constexpr PtrType operator->() const noexcept { return m_Ptr; }

		/**
		 * @brief Indirection operator. Dereferences element.
		 * @return Reference to element.
		 */
		constexpr RefType operator*() const noexcept { return *m_Ptr; }

		/**
		 * @brief Returns the raw pointer to the element.
		 * @return Raw pointer.
		 */
		constexpr PtrType Ptr() const noexcept { return m_Ptr; }

	private:
		PtrType m_Ptr; ///< Underlying raw pointer to element in buffer.
	};
}
