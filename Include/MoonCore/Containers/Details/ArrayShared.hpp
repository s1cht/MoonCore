#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Utility/Iterator.hpp"

namespace MC
{
	template<auto ArraySize>
	    requires Concept::CountedScopedEnum<decltype(ArraySize)> || Concept::Integral<decltype(ArraySize)>
	consteval usize ToArraySize() noexcept
	{
		if constexpr (Concept::Integral<decltype(ArraySize)>)
			return static_cast<usize>(ArraySize);
		else
			return static_cast<usize>(ToUnderlying(ArraySize));
	}

	template<typename Container, bool IsConst = false>
	class ArrayIterator
	{
	public:
		using ValueType = typename Container::ValueType;
		using DiffType = iptr;
		using PtrType = Conditional<IsConst, const ValueType*, ValueType*>;
		using RefType = Conditional<IsConst, const ValueType&, ValueType&>;
		using Category = Iterator::Tags::RandomAccess;

	public:
		constexpr ArrayIterator() noexcept : m_Ptr(nullptr) {}
		constexpr ArrayIterator(PtrType ptr) noexcept : m_Ptr(ptr) {}

	public:
		constexpr ArrayIterator& operator+=(usize index) noexcept { m_Ptr += index; return *this; }
		constexpr ArrayIterator& operator-=(usize index) noexcept { m_Ptr -= index; return *this; }

		constexpr ArrayIterator operator+(usize index) const noexcept { return ArrayIterator(m_Ptr + index); }
		constexpr ArrayIterator operator-(usize index) const noexcept { return ArrayIterator(m_Ptr - index); }

		constexpr ArrayIterator& operator++() noexcept { ++m_Ptr; return *this; }
		constexpr ArrayIterator operator++(int) noexcept { ArrayIterator temp = *this; ++m_Ptr; return temp; }

		constexpr ArrayIterator& operator--() noexcept { --m_Ptr; return (*this); }
		constexpr ArrayIterator operator--(int) noexcept { ArrayIterator temp = *this; --m_Ptr; return temp; }

		constexpr DiffType operator-(const ArrayIterator& it) const noexcept { return m_Ptr - it.m_Ptr; }

		constexpr bool operator==(const ArrayIterator& it) const noexcept { return m_Ptr == it.m_Ptr; }
		constexpr bool operator!=(const ArrayIterator& it) const noexcept { return m_Ptr != it.m_Ptr; }
		constexpr bool operator<(const ArrayIterator& it) const noexcept { return m_Ptr < it.m_Ptr; }
		constexpr bool operator>(const ArrayIterator& it) const noexcept { return m_Ptr > it.m_Ptr; }

		constexpr RefType operator[](usize index) const noexcept { return *(m_Ptr + index); }
		constexpr PtrType operator->() const noexcept { return m_Ptr; }
		constexpr RefType operator*() const noexcept { return *m_Ptr; }

		constexpr PtrType Ptr() const noexcept { return m_Ptr; }

	private:
		PtrType m_Ptr;
	};
}
