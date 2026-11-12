#pragma once
#include "MoonCore/Core.hpp"

namespace MC::Concept
{
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