#pragma once
#include "MoonCore/Containers/Details/ArrayShared.hpp"
#include "MoonCore/Utility/Algorithm.hpp"
#include "MoonCore/Memory/Memory.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "MoonCore/Utility/InitializerList.hpp"
#include "MoonCore/Utility/Nullable.hpp"

/**
 * @file FixedArray.hpp
 * @brief Fixed-size compile-time array container.
 * @ingroup seq_containers
 */

namespace MC
{
	/**
	 * @class FixedArrayImpl
	 * @brief Fixed-size compile-time array with contiguous in-place storage.
	 *
	 * `FixedArrayImpl<T, ArraySize>` is a constant-size sequence container that wraps a standard
	 * C-style array with standard container interfaces, iterators, and bounds checking.
	 * Its capacity and size are fixed at compile time and requires zero dynamic heap allocations.
	 *
	 * Supports indexing via both integer types and scoped enums (`Concept::ScopedEnum`).
	 *
	 * @tparam T The element type stored in the array. Must satisfy `IsNothrowDestructible<T>`.
	 * @tparam ArraySize Compile-time constant specifying the number of elements.
	 *
	 * @code
	 * MC::FixedArray<int, 4> coords = { 10, 20, 30, 40 };
	 * coords[0] = 5;
	 * for (int c : coords) {
	 *     // Process coordinate
	 * }
	 * @endcode
	 */
	template<typename T, usize ArraySize>
		requires IsNothrowDestructible<T>
	class FixedArrayImpl
	{
	public:
		using ValueType = T;                                           ///< Element type.
		using PtrType = ValueType*;                                    ///< Pointer to element.
		using ConstPtrType = const ValueType*;                          ///< Pointer to const element.
		using RefType = ValueType&;                                    ///< Reference to element.
		using ConstRefType = const ValueType&;                          ///< Reference to const element.

		using Iterator = ArrayIterator<FixedArrayImpl, false>;          ///< Random-access iterator.
		using ConstIterator = ArrayIterator<FixedArrayImpl, true>;      ///< Const random-access iterator.
		using ReverseIterator = MC::Iterator::ReverseIterator<Iterator>;///< Reverse iterator.
		using ConstReverseIterator = MC::Iterator::ReverseIterator<ConstIterator>; ///< Const reverse iterator.

	public:
		static constexpr usize FixedSize = ArraySize;                  ///< Number of elements in array.

	public:
		/**
		 * @brief Default constructor. Leaves elements uninitialized or default-initialized.
		 */
		constexpr FixedArrayImpl() noexcept {}

		/**
		 * @brief Constructs a fixed array from an initializer list.
		 * @param other Initializer list whose size must not exceed @p ArraySize.
		 * @note Asserts `other.size() <= ArraySize` in debug builds.
		 */
		constexpr FixedArrayImpl(const Utility::InitializerList<ValueType>& other) noexcept
		{
			MOON_CHECK(other.size() <= ArraySize, "initializer_list overflows FixedArrayImpl");
			for (usize i = 0; i < FixedSize; i++)
				m_Data[i] = other.data()[i];
		}

		/**
		 * @brief Copy constructor. Copies each element from @p other.
		 * @param other Source array to copy.
		 */
		constexpr FixedArrayImpl(const FixedArrayImpl& other) noexcept
		{
			for (usize i = 0; i < FixedSize; i++)
				m_Data[i] = other.m_Data[i];
		}

		/**
		 * @brief Move constructor. Moves each element from @p other.
		 * @param other Source array to move from.
		 */
		constexpr FixedArrayImpl(FixedArrayImpl&& other) noexcept
		{
			for (usize i = 0; i < FixedSize; i++)
				m_Data[i] = Move(other.m_Data[i]);
		}

		/**
		 * @brief Default destructor.
		 */
		constexpr ~FixedArrayImpl() noexcept = default;

	public:
		/**
		 * @brief Returns the constant number of elements in the array.
		 * @return Element count (`ArraySize`).
		 */
		static constexpr usize Size() noexcept { return FixedSize; }

		/**
		 * @brief Returns the total capacity, which equals @p ArraySize.
		 * @return Element capacity.
		 */
		static constexpr usize Capacity() noexcept { return FixedSize; }

		/**
		 * @brief Returns a const pointer to the raw data buffer.
		 * @return Const pointer to buffer.
		 */
		constexpr ConstPtrType Data() const noexcept { return m_Data; }

		/**
		 * @brief Returns a mutable pointer to the raw data buffer.
		 * @return Pointer to buffer.
		 */
		constexpr PtrType Data() noexcept { return m_Data; }
		
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
		constexpr RefType Back() noexcept { return *(m_Data + FixedSize - 1); }

		/**
		 * @brief Accesses the last element (const overload).
		 * @return Const reference to last element.
		 */
		constexpr ConstRefType Back() const noexcept { return *(m_Data + FixedSize - 1); }

	public:
		/** @brief Returns an iterator to the beginning. */
		constexpr Iterator Begin() noexcept { return Iterator(m_Data); }
		/** @brief Returns a const iterator to the beginning. */
		constexpr ConstIterator Begin() const noexcept { return ConstIterator(m_Data); }
		/** @brief Returns a const iterator to the beginning. */
		constexpr ConstIterator CBegin() const noexcept { return ConstIterator(m_Data); }
		/** @brief Returns an iterator to the end. */
		constexpr Iterator End() noexcept { return Iterator(m_Data + FixedSize); }
		/** @brief Returns a const iterator to the end. */
		constexpr ConstIterator End() const noexcept { return ConstIterator(m_Data + FixedSize); }
		/** @brief Returns a const iterator to the end. */
		constexpr ConstIterator CEnd() const noexcept { return ConstIterator(m_Data + FixedSize); }

		/** @brief Returns a reverse iterator to the beginning of reversed sequence. */
		constexpr ReverseIterator RBegin() noexcept { return FixedSize > 0 ? ReverseIterator(m_Data + FixedSize - 1) : REnd(); }
		/** @brief Returns a const reverse iterator to the beginning of reversed sequence. */
		constexpr ConstReverseIterator RBegin() const noexcept { return FixedSize > 0 ? ReverseIterator(m_Data + FixedSize - 1) : REnd(); }
		/** @brief Returns a const reverse iterator to the beginning of reversed sequence. */
		constexpr ConstReverseIterator CRBegin() const noexcept { return FixedSize > 0 ? ConstReverseIterator(m_Data + FixedSize - 1) : CREnd(); }
		/** @brief Returns a reverse iterator to the end of reversed sequence. */
		constexpr ReverseIterator REnd() noexcept { return ReverseIterator(m_Data - 1); }
		/** @brief Returns a const reverse iterator to the end of reversed sequence. */
		constexpr ConstReverseIterator REnd() const noexcept { return ReverseIterator(m_Data - 1); }
		/** @brief Returns a const reverse iterator to the end of reversed sequence. */
		constexpr ConstReverseIterator CREnd() const noexcept { return ConstReverseIterator(m_Data - 1); }

	public:
		/** @brief Range-for compatible begin iterator. */
		constexpr Iterator begin() noexcept { return Begin(); }
		/** @brief Range-for compatible const begin iterator. */
		constexpr ConstIterator begin() const noexcept { return CBegin(); }
		/** @brief Range-for compatible const begin iterator. */
		constexpr ConstIterator cbegin() const noexcept { return CBegin(); }
		
		/** @brief Range-for compatible end iterator. */
	    constexpr Iterator end() noexcept { return End(); }
		/** @brief Range-for compatible const end iterator. */
		constexpr ConstIterator end() const noexcept { return CEnd(); }
		/** @brief Range-for compatible const end iterator. */
		constexpr ConstIterator cend() const noexcept { return CEnd(); }

		/** @brief Range-for compatible reverse begin iterator. */
		constexpr ReverseIterator rbegin() noexcept { return RBegin(); }
		/** @brief Range-for compatible const reverse begin iterator. */
		constexpr ConstReverseIterator rbegin() const noexcept { return RBegin(); }
		/** @brief Range-for compatible const reverse begin iterator. */
		constexpr ConstReverseIterator crbegin() const noexcept { return CRBegin(); }
		/** @brief Range-for compatible reverse end iterator. */
		constexpr ReverseIterator rend() noexcept { return REnd(); }
		/** @brief Range-for compatible const reverse end iterator. */
		constexpr ConstReverseIterator rend() const noexcept { return REnd(); }
		/** @brief Range-for compatible const reverse end iterator. */
		constexpr ConstReverseIterator crend() const noexcept { return CREnd(); }

	public:
		/**
		 * @brief Copy assignment operator.
		 * @param other Source array.
		 * @return Reference to this array.
		 */
		constexpr FixedArrayImpl& operator=(const FixedArrayImpl& other) noexcept
		{
			if (this != &other)
			{
				for (usize i = 0; i < FixedSize; i++)
					Memory::Construct(&m_Data[i], other.m_Data[i]);
			}
			return *this;
		}

		/**
		 * @brief Move assignment operator.
		 * @param other Source array.
		 * @return Reference to this array.
		 */
		constexpr FixedArrayImpl& operator=(FixedArrayImpl&& other) noexcept
		{
			if (this != &other)
			{
				for (usize i = 0; i < FixedSize; i++)
					Memory::Construct(&m_Data[i], Move(other.m_Data[i]));
			}
			return *this;
		}

		/**
		 * @brief Subscript access operator.
		 * @param index 0-based element index.
		 * @return Mutable reference to element.
		 * @note Asserts `index < FixedSize` in debug builds.
		 */
		constexpr RefType operator[](const usize index) noexcept
		{
			MOON_CHECK(index < FixedSize, "Index in array is out of range!");
			return m_Data[index];
		}

		/**
		 * @brief Subscript access operator (const overload).
		 * @param index 0-based element index.
		 * @return Const reference to element.
		 * @note Asserts `index < FixedSize` in debug builds.
		 */
		constexpr ConstRefType operator[](const usize index) const noexcept
		{
			MOON_CHECK(index < FixedSize, "Index in array is out of range!");
			return m_Data[index];
		}

		/**
		 * @brief Subscript access operator using scoped enum index.
		 * @tparam Enum Scoped enum type satisfying `Concept::ScopedEnum`.
		 * @param index Enum value cast to integer index.
		 * @return Mutable reference to element.
		 */
		template<Concept::ScopedEnum Enum>
		constexpr RefType operator[](const Enum index) noexcept
		{
			MOON_CHECK(static_cast<usize>(index) < FixedSize, "Index in array is out of range!");
			return m_Data[static_cast<usize>(index)];
		}

		/**
		 * @brief Subscript access operator using scoped enum index (const overload).
		 * @tparam Enum Scoped enum type satisfying `Concept::ScopedEnum`.
		 * @param index Enum value cast to integer index.
		 * @return Const reference to element.
		 */
		template<Concept::ScopedEnum Enum>
		constexpr ConstRefType operator[](const Enum index) const noexcept
		{
			MOON_CHECK(static_cast<usize>(index) < FixedSize, "Index in array is out of range!");
			return m_Data[static_cast<usize>(index)];
		}

	private:
		ValueType m_Data[ArraySize]; ///< In-place contiguous element buffer.
	};

	/**
	 * @brief Convenient alias for `FixedArrayImpl`, accepting integer constants or counted scoped enums.
	 * @tparam T Stored element type.
	 * @tparam ArraySize Capacity as an integer or scoped enum with `Count`.
	 */
	template<typename T, auto ArraySize>
	using FixedArray = FixedArrayImpl<T, ToArraySize<ArraySize>()>;
}
