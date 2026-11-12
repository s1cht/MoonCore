#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Utility/Iterator.hpp"

namespace MC
{
	template<Concept::InputIteratorType Iterator, typename T = Iterator::ValueType>
	constexpr Iterator Find(Iterator first, Iterator last, const T& value) noexcept
	{
		for (Iterator it = first; it != last; ++it)
			if (*it == value)
				return it;
		return last;
	}

	template<Concept::InputIteratorType Iterator, typename T = Iterator::ValueType>
	constexpr Iterator BinaryFind(Iterator first, Iterator last, const T& value) noexcept
	{
		while (first < last)
		{
			Iterator mid = first + (last - first) / 2;

			if (*mid == value)
				return mid;
			if (*mid < value)
				first = mid + 1;
			else
				last = mid;
		}

		return last;
	}

	template<Concept::InputIteratorType FirstIterator, Concept::InputIteratorType SecondIterator>
        requires Concept::HasEqualCompare<typename FirstIterator::ValueType, typename SecondIterator::ValueType>
	constexpr FirstIterator FindFirstOf(FirstIterator first, FirstIterator last,
		SecondIterator firstF, SecondIterator lastF) noexcept
	{
		for (; first != last; ++first)
			for (SecondIterator it = firstF; it != lastF; ++it)
				if (*first == *it)
					return first;
		return last;
	}

	template <Concept::HasLessCompareOperation T>
	static constexpr T Max(const T& a, const T& b) noexcept
	{
		return (a < b) ? b : a;
	}

	template <Concept::HasGreaterCompareOperation T>
    static constexpr T Min(const T& a, const T& b) noexcept
	{
		return (a > b) ? b : a;
	}

	template<typename T>
	static constexpr T	Clamp(T min, T max, T val)
	{
		return Max(min, Min(val, max));
	}
}