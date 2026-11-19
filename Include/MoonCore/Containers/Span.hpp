#pragma once
#include "MoonCore/Containers/FixedArray.hpp"
#include "MoonCore/Containers/Details/ArrayShared.hpp"
#include "MoonCore/Utility/Algorithm.hpp"
#include "MoonCore/Memory/Memory.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "MoonCore/Utility/InitializerList.hpp"

/**
 * @file Span.hpp
 * @brief Non-owning view over a contiguous sequence of elements.
 * @ingroup seq_containers
 */

namespace MC
{
	/**
	 * @brief Sentinel extent value meaning "size is known only at runtime".
	 *
	 * A `SpanImpl<T, DynamicExtent>` stores its element count as a member. Any other
	 * extent makes the count part of the type, and the span stores only a pointer.
	 */
	inline constexpr usize DynamicExtent = NumericLimit<usize>::Max;

    namespace Details
    {
		/**
		 * @brief Converts a contiguous iterator or raw pointer into a raw element pointer.
		 * @tparam T  Element type the resulting pointer points to.
		 * @tparam It Iterator type. Raw pointers are returned as-is; other iterators must expose `Ptr()`.
		 * @param iterator Iterator to unwrap.
		 * @return Raw pointer to the element @p iterator refers to.
		 */
		template<typename T, Concept::OutputIteratorType It>
		inline constexpr T* UnwrapIterator(It&& iterator) noexcept
		{
			if constexpr (IsPointer<Decay<It>>) return iterator;
			else
			{
				return iterator.Ptr();
			}
		}

		/**
		 * @brief Storage for a span with a compile-time extent.
		 *
		 * Holds only the data pointer. The size is `static constexpr`, so a
		 * fixed-extent span is pointer-sized.
		 */
		template<typename T, usize Extent>
		struct SpanData
		{
			using PtrType = T*;

			constexpr SpanData() noexcept = default;
			constexpr SpanData(const PtrType ptr, usize) noexcept : m_Ptr(ptr) {}

			PtrType m_Ptr = nullptr;
			static constexpr usize m_Size = Extent;
		};

		/**
		 * @brief Storage for a span with a runtime extent.
		 *
		 * Holds both the data pointer and the element count.
		 */
		template<typename T>
		struct SpanData<T, DynamicExtent>
        {
			using PtrType = T*;

			constexpr SpanData() noexcept = default;
			constexpr explicit SpanData(const PtrType ptr, const usize size) noexcept : m_Ptr(ptr), m_Size(size) {}

		    PtrType m_Ptr = nullptr;
			usize m_Size = 0;
        };
    }

	/**
	 * @class SpanImpl
	 * @brief Non-owning view over a contiguous sequence of `T`.
	 *
	 * `SpanImpl<T, Extent>` refers to elements that live somewhere else, such as a C array,
	 * `FixedArray`, `Array` or any contiguous range. It never allocates, copies or
	 * destroys the elements. Copying a span copies the view (pointer and size), not the data.
	 *
	 * The extent can be one of two kinds:
	 * - **Static** (`Extent != DynamicExtent`): the size is part of the type and the span
	 *   stores only a pointer.
	 * - **Dynamic** (`Extent == DynamicExtent`, the default): the size is stored at runtime
	 *   alongside the pointer.
	 *
	 * Constness of the viewed elements is controlled by `T`. `SpanImpl<const T>` gives read-only
	 * access, while `SpanImpl<T>` allows writing through the view even when the span itself is `const`.
	 *
	 * Elements can be indexed with integer types and with scoped enums (`Concept::ScopedEnum`).
	 *
	 * @warning The span does not extend the lifetime of the viewed storage. Using a span after
	 *          the underlying container is destroyed or reallocated is undefined behaviour.
	 *
	 * @tparam T      Element type being viewed. Add `const` to make the view read-only.
	 * @tparam Extent Number of elements known at compile time, or `DynamicExtent` (the default).
	 *
	 * @code
	 * int raw[4] = { 10, 20, 30, 40 };
	 * MC::FixedArray<int, 3> fixed = { 1, 2, 3 };
	 *
	 * MC::SpanImpl<int>       all(raw);    // dynamic extent, Size() == 4
	 * MC::SpanImpl<int, 3>    view(fixed); // static extent, pointer-sized
	 * MC::SpanImpl<const int> readOnly(raw);
	 *
	 * all[0] = 5; // writes through to raw[0]
	 * for (int value : readOnly) {
	 *     // Process value
	 * }
	 * @endcode
	 */
	template<typename T, usize Extent = DynamicExtent>
	class SpanImpl : private Details::SpanData<T, Extent>
	{
		using Base = Details::SpanData<T, Extent>;
		using Base::m_Ptr;
		using Base::m_Size;

	public:
		using ValueType = T;                                           ///< Element type.
		using PtrType = ValueType*;                                    ///< Pointer to element.
		using ConstPtrType = const ValueType*;                          ///< Pointer to const element.
		using RefType = ValueType&;                                    ///< Reference to element.
		using ConstRefType = const ValueType&;                          ///< Reference to const element.

		using Iterator = ArrayIterator<SpanImpl, false>;          ///< Random-access iterator.
		using ConstIterator = ArrayIterator<SpanImpl, true>;      ///< Const random-access iterator.
		using ReverseIterator = MC::Iterator::ReverseIterator<Iterator>;///< Reverse iterator.
		using ConstReverseIterator = MC::Iterator::ReverseIterator<ConstIterator>; ///< Const reverse iterator.

	public:
		/**
		 * @brief Constructs an empty span (null pointer, size 0 for dynamic extent).
		 */
		constexpr SpanImpl() noexcept = default;

		/**
		 * @brief Constructs a span over @p count elements starting at @p first.
		 * @tparam It Contiguous iterator or raw pointer type.
		 * @param first Iterator to the first element of the viewed range.
		 * @param count Number of elements to view. Must equal `Extent` for a static-extent span.
		 * @note Explicit for static-extent spans.
		 */
		template<Concept::OutputIteratorType It>
        constexpr explicit(Extent != DynamicExtent) SpanImpl(It first, usize count) noexcept
			: Base(Details::UnwrapIterator<T>(first), count)
		{
			if constexpr (Extent != DynamicExtent)
				MOON_CHECK(Extent == count, "Count should be equal as Extent!");
		}

		/**
		 * @brief Constructs a span over the half-open range `[first, last)`.
		 * @tparam FirstIt Contiguous iterator or raw pointer type for the start.
		 * @tparam LastIt  Contiguous iterator or raw pointer type for the end.
		 * @param first Iterator to the first element.
		 * @param last  Iterator one past the last element.
		 * @note `last - first` must equal `Extent` for a static-extent span. Explicit for static-extent spans.
		 */
		template<Concept::OutputIteratorType FirstIt, Concept::OutputIteratorType LastIt>
		constexpr explicit(Extent != DynamicExtent) SpanImpl(FirstIt first, LastIt last) noexcept(noexcept(last - first))
			: Base(Details::UnwrapIterator<T>(first), Details::UnwrapIterator<T>(last) - Details::UnwrapIterator<T>(first))
		{
			if constexpr (Extent != DynamicExtent)
				MOON_CHECK(Extent == Details::UnwrapIterator<T>(last) - Details::UnwrapIterator<T>(first), "Count should be equal as Extent!");
		}

		/**
		 * @brief Constructs a span viewing an entire C-style array.
		 * @tparam Size Length of the array, deduced. Must equal `Extent` unless the extent is dynamic.
		 * @param array Array to view.
		 */
		template<usize Size>
			requires (Extent == DynamicExtent || Extent == Size)
		constexpr SpanImpl(ValueType (&array)[Size]) noexcept : Base(array, Size) {}

		/**
		 * @brief Constructs a span viewing an entire `FixedArray`.
		 * @tparam U    Element type of the array. `U(*)[]` must convert to `T(*)[]`, which allows
		 *              adding `const` but rejects unrelated or derived-to-base conversions.
		 * @tparam Size Length of the array, deduced. Must equal `Extent` unless the extent is dynamic.
		 * @param array Array to view.
		 */
		template<typename U, usize Size>
            requires (Extent == DynamicExtent || Extent == Size) && IsConvertible<U (*)[], ValueType(*)[]>
		constexpr SpanImpl(FixedArray<U, Size>& array) noexcept : Base(array.Data(), Size) {}

		/**
		 * @brief Constructs a span viewing a contiguous range.
		 * @tparam Range Container type exposing `Begin()` and `End()` over contiguous storage.
		 * @param range Range to view. Must outlive the span.
		 * @note Explicit for static-extent spans.
		 */
		template<typename Range>
        constexpr explicit (Extent != DynamicExtent) SpanImpl(Range&& range) noexcept 
            : Base(range.Begin(), range.End() - range.Begin()) {}

		/**
		 * @brief Converting constructor from another span.
		 *
		 * Creates a view of the same elements, optionally adding `const` or converting
		 * between static and dynamic extents.
		 *
		 * @tparam U    Element type of the source span. `U(*)[]` must convert to `T(*)[]`.
		 * @tparam Size Extent of the source span. Must match `Extent` unless either is dynamic.
		 * @param other SpanImpl to view the elements of.
		 * @note Explicit when converting from a dynamic extent into a static one.
		 */
		template<typename U, usize Size>
            requires (Size == Extent || Size == DynamicExtent || Extent == DynamicExtent) && IsConvertible<U(*)[], ValueType(*)[]>
		constexpr explicit(Size != DynamicExtent && Extent != DynamicExtent) SpanImpl(const SpanImpl& other) noexcept
			: Base(other.Data(), other.Size()) {}

		/**
		 * @brief Destructor. Does not touch the viewed elements.
		 */
		constexpr ~SpanImpl() noexcept = default;

	public:
		/**
		 * @brief Returns the number of viewed elements.
		 * @return `Extent` for a static-extent span, or the runtime count otherwise.
		 */
		constexpr usize Size() noexcept { return m_Size; }

		/**
		 * @brief Returns a const pointer to the first viewed element.
		 * @return Const pointer to the data, or `nullptr` for a default-constructed span.
		 */
		constexpr ConstPtrType Data() const noexcept { return m_Ptr; }

		/**
		 * @brief Returns a pointer to the first viewed element.
		 * @return Pointer to the data, or `nullptr` for a default-constructed span.
		 */
		constexpr PtrType Data() noexcept { return m_Ptr; }

		/**
		 * @brief Accesses the first viewed element.
		 * @return Mutable reference to the first element.
		 * @pre `Size() > 0`.
		 */
		constexpr RefType Front() noexcept { return *m_Ptr; }

		/**
		 * @brief Accesses the first viewed element (const overload).
		 * @return Const reference to the first element.
		 * @pre `Size() > 0`.
		 */
		constexpr ConstRefType Front() const noexcept { return *m_Ptr; }

		/**
		 * @brief Accesses the last viewed element.
		 * @return Mutable reference to the last element.
		 * @pre `Size() > 0`.
		 */
		constexpr RefType Back() noexcept { return *(m_Ptr + m_Size - 1); }

		/**
		 * @brief Accesses the last viewed element (const overload).
		 * @return Const reference to the last element.
		 * @pre `Size() > 0`.
		 */
		constexpr ConstRefType Back() const noexcept { return *(m_Ptr + m_Size - 1); }

	public:
		/** @brief Returns an iterator to the beginning. */
		constexpr Iterator Begin() noexcept { return Iterator(m_Ptr); }
		/** @brief Returns a const iterator to the beginning. */
		constexpr ConstIterator Begin() const noexcept { return ConstIterator(m_Ptr); }
		/** @brief Returns a const iterator to the beginning. */
		constexpr ConstIterator CBegin() const noexcept { return ConstIterator(m_Ptr); }
		/** @brief Returns an iterator to the end. */
		constexpr Iterator End() noexcept { return Iterator(m_Ptr + m_Size); }
		/** @brief Returns a const iterator to the end. */
		constexpr ConstIterator End() const noexcept { return ConstIterator(m_Ptr + m_Size); }
		/** @brief Returns a const iterator to the end. */
		constexpr ConstIterator CEnd() const noexcept { return ConstIterator(m_Ptr + m_Size); }

		/** @brief Returns a reverse iterator to the beginning of reversed sequence. */
		constexpr ReverseIterator RBegin() noexcept { return m_Size > 0 ? ReverseIterator(m_Ptr + m_Size - 1) : REnd(); }
		/** @brief Returns a const reverse iterator to the beginning of reversed sequence. */
		constexpr ConstReverseIterator RBegin() const noexcept { return m_Size > 0 ? ReverseIterator(m_Ptr + m_Size - 1) : REnd(); }
		/** @brief Returns a const reverse iterator to the beginning of reversed sequence. */
		constexpr ConstReverseIterator CRBegin() const noexcept { return m_Size > 0 ? ConstReverseIterator(m_Ptr + m_Size - 1) : CREnd(); }
		/** @brief Returns a reverse iterator to the end of reversed sequence. */
		constexpr ReverseIterator REnd() noexcept { return ReverseIterator(m_Ptr - 1); }
		/** @brief Returns a const reverse iterator to the end of reversed sequence. */
		constexpr ConstReverseIterator REnd() const noexcept { return ReverseIterator(m_Ptr - 1); }
		/** @brief Returns a const reverse iterator to the end of reversed sequence. */
		constexpr ConstReverseIterator CREnd() const noexcept { return ConstReverseIterator(m_Ptr - 1); }

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
		 * @brief Copy assignment. Rebinds this span to view the same elements as @p other.
		 * @return Reference to this span.
		 * @note No elements are copied. Only the pointer (and size) change.
		 */
		constexpr SpanImpl& operator=(const SpanImpl&) noexcept = default;
		/**
		 * @brief Move assignment. Same as copy assignment, because a span owns nothing.
		 * @return Reference to this span.
		 */
		constexpr SpanImpl& operator=(SpanImpl&&) noexcept = default;

		/**
		 * @brief Subscript access operator.
		 * @param index 0-based element index.
		 * @return Mutable reference to element.
		 * @note Asserts `index < m_Size` in debug builds.
		 */
		constexpr RefType operator[](const usize index) noexcept
		{
			MOON_CHECK(index < m_Size, "Index in array is out of range!");
			return m_Ptr[index];
		}

		/**
		 * @brief Subscript access operator (const overload).
		 * @param index 0-based element index.
		 * @return Const reference to element.
		 * @note Asserts `index < m_Size` in debug builds.
		 */
		constexpr ConstRefType operator[](const usize index) const noexcept
		{
			MOON_CHECK(index < m_Size, "Index in array is out of range!");
			return m_Ptr[index];
		}

		/**
		 * @brief Subscript access operator using scoped enum index.
		 * @tparam Enum Scoped enum type satisfying `Concept::ScopedEnum`.
		 * @param index Enum value cast to integer index.
		 * @return Mutable reference to element.
		 * @note Asserts `index < m_Size` in debug builds.
		 */
		template<Concept::ScopedEnum Enum>
		constexpr RefType operator[](const Enum index) noexcept
		{
			MOON_CHECK(static_cast<usize>(index) < m_Size, "Index in array is out of range!");
			return m_Ptr[static_cast<usize>(index)];
		}

		/**
		 * @brief Subscript access operator using scoped enum index (const overload).
		 * @tparam Enum Scoped enum type satisfying `Concept::ScopedEnum`.
		 * @param index Enum value cast to integer index.
		 * @return Const reference to element.
		 * @note Asserts `index < m_Size` in debug builds.
		 */
		template<Concept::ScopedEnum Enum>
		constexpr ConstRefType operator[](const Enum index) const noexcept
		{
			MOON_CHECK(static_cast<usize>(index) < m_Size, "Index in array is out of range!");
			return m_Ptr[static_cast<usize>(index)];
		}
	};

	/**
	 * @brief Convenient alias for `SpanImpl`, accepting integer constants or counted scoped enums.
	 * @tparam T Stored element type.
	 * @tparam Extent Extent as an integer or scoped enum with `Count`.
	 */
	template<typename T, auto Extent = DynamicExtent>
	using Span = SpanImpl<T, ToArraySize<Extent>()>;
}