#pragma once
#include "MoonCore/Core.hpp"

namespace MC::Concept
{
    template<class T, typename ResultType, typename... ArgTypes>
    concept IsFunctor = requires(T functor, ArgTypes&&... args)
    {   
        { functor(Forward<ArgTypes>(args)...) } -> ConvertibleTo<ResultType>;
    };

    template<typename T>
    concept CompareFunctor = requires { typename T::InputType; } && IsFunctor<T, bool, const typename T::InputType&, const typename T::InputType&>;
}

namespace MC::Callables
{
    template<Concept::HasLessCompareOperation T>
    struct Less
    {
        using InputType = T;

        constexpr bool operator()(const T& first, const T& second) const noexcept(noexcept(first < second))
        {
            return first < second;
        }
    };

    template<Concept::HasGreaterCompareOperation T>
    struct Greater
    {
        using InputType = T;

        constexpr bool operator()(const T& first, const T& second) const  noexcept(noexcept(first > second))
        {
            return first > second;
        }
    };
}