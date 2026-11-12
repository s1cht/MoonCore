#pragma once
#include "MoonCore/Types.hpp"
#include "MoonCore/Utility/Tuple.hpp"

namespace MC::Callables
{
    template<u64 Place> 
        requires (Place > 0)
    struct PlaceholderT { static constexpr u64 GetPlace() noexcept { return Place; } };

    template<u64 Place> static constexpr PlaceholderT<Place> Placeholder = PlaceholderT<Place>();

    template<typename T> constexpr bool IsPlaceholder = false;
    template<u64 N> constexpr bool IsPlaceholder<PlaceholderT<N>> = true;

    template <typename Callable, typename... BoundArgTypes>
    class Binder
    {
    public:
        constexpr Binder(Callable function, BoundArgTypes... args) noexcept
            : m_Function(Move(function)), m_BoundArgs(Move(args)...)
        {}

    public:
        template<typename... ArgTypes>
        constexpr decltype(auto) operator()(ArgTypes&&... args) const
        {
            return Apply(Utility::MakeIndexSequence<sizeof...(BoundArgTypes)>{}, Forward<ArgTypes>(args)...);
        }

    private:
        template <usize... Indices, typename... ArgTypes>
        constexpr decltype(auto) Apply(Utility::IndexSequence<Indices...>, ArgTypes&&... args) const
        {
            auto forwarded = Utility::ForwardAsTuple(Forward<ArgTypes>(args)...);
            return Invoke(m_Function, Resolve(m_BoundArgs.template Member<Indices>(), forwarded)...);
        }

        template <typename BoundArg, typename ForwardedTuple>
        constexpr decltype(auto) Resolve(BoundArg& bound, ForwardedTuple& forwarded) const noexcept
        {
            using Bare = RemoveCV<RemoveReference<BoundArg>>;
            if constexpr (IsPlaceholder<Bare>)
            {
                using Element = typename ForwardedTuple::template MemberType<Bare::GetPlace() - 1>;
                return static_cast<Element&&>(forwarded.template Member<Bare::GetPlace() - 1>());
            }
            else
                return bound;
        }

    private:
        Callable m_Function;
        Utility::Tuple<BoundArgTypes...> m_BoundArgs;
    };

    template <typename Callable>
    class Binder<Callable>
    {
    public:
        constexpr Binder(Callable function) noexcept
            : m_Function(Move(function))
        {}

    public:
        template<typename... ArgTypes>
        constexpr decltype(auto) operator()(ArgTypes&&... args) const
        {
            return Invoke(m_Function, Forward<ArgTypes>(args)...);
        }

    private:
        Callable m_Function;
    };

    template <typename Callable, typename... ArgTypes>
    constexpr auto Bind(Callable&& function, ArgTypes&&... args) noexcept
    {
        return Binder<Decay<Callable>, Decay<ArgTypes>...>(
            Forward<Callable>(function), Forward<ArgTypes>(args)...);
    }
}