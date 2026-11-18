#pragma once
#include "MoonCore/Core.hpp"

/**
 * @file HeapShared.hpp
 * @brief Concepts and interfaces for heap-based priority queue containers.
 * @ingroup container_adapters
 */

namespace MC::Concept
{
    /**
     * @concept HeapType
     * @brief Concept describing requirements for a binary heap container.
     *
     * Requires types `ContainerType`, `ValueType`, `RefType`, `ConstRefType`,
     * noexcept construction from container, and methods `Size()`, `Empty()`, `Top()`, `Insert()`, `Emplace()`, and `Pop()`.
     *
     * @tparam T The heap container type being validated.
     */
    template <class T>
    concept HeapType = requires
    {
        typename T::ContainerType;
        typename T::ValueType;
        typename T::RefType;
        typename T::ConstRefType;
    }
    && IsNothrowConstructible<T, const typename T::ContainerType&>
    && IsNothrowConstructible<T, typename T::ContainerType&&>
    && requires(T heap, const T constHeap, const typename T::ValueType& lValue, typename T::ValueType&& rValue)
    {
        { heap.Size() }     -> ConvertibleTo<usize>;
        { heap.Empty() }    -> ConvertibleTo<bool>;
        { heap.Top() }      -> ConvertibleTo<typename T::RefType>;
        { constHeap.Top() } -> ConvertibleTo<typename T::ConstRefType>;

        { heap.Insert(lValue) };
        { heap.Insert(rValue) };
        { heap.Emplace() } -> ConvertibleTo<typename T::RefType>;
        { heap.Pop() };
    };
}
