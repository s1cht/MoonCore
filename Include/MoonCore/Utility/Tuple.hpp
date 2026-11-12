#pragma once
#include "Sequences.hpp"
#include "MoonCore/Types.hpp"

namespace MC::Utility
{
    namespace Helper
    {
        template <usize Index, typename... TypeList> struct TupleMember;

        template <usize CurrentIndex, typename Head, typename... Tail>
        struct TupleMember<CurrentIndex, Head, Tail...>
        {
            Head Member;
            TupleMember<CurrentIndex + 1, Tail...> NextMember;

            static constexpr usize Index() noexcept { return CurrentIndex; }

            constexpr TupleMember() noexcept = default;
            template<typename H, typename... T>
            constexpr TupleMember(H&& head, T&&... tail) noexcept
                : Member(Forward<H>(head)), NextMember(Forward<T>(tail)...) {}
        };

        template <usize CurrentIndex>
        struct TupleMember<CurrentIndex>
        {
            static constexpr usize Index() noexcept { return CurrentIndex; }
        };
    }

    template <typename... Args>
    class Tuple
    {
    public:
        using Types = Typelist<Args...>;
        template<usize Index> using MemberType = Types::template Type<Index>;
        using HeadType = Helper::TupleMember<0, Args...>;

    public:
        constexpr Tuple() noexcept = default;
        constexpr Tuple(const Args&... args) noexcept requires (sizeof...(Args) > 0)
            : Members(args...)
        {
        }
        template<typename... ArgTypes>
            requires (sizeof...(ArgTypes) == sizeof...(Args)) && (sizeof...(ArgTypes) > 0)
                     && (sizeof...(ArgTypes) != 1 || (!IsSame<Decay<ArgTypes>, Tuple> && ...))
                     && (IsConstructible<Args, ArgTypes&&> && ...)
        constexpr Tuple(ArgTypes&&... args) noexcept
            : Members(Forward<ArgTypes>(args)...) {}

    public:
        template<usize Index>
        constexpr MemberType<Index>& Member() noexcept
        {
            return GetMemberFromHolder<Index>(Members);
        }

        template<usize Index>
        constexpr const MemberType<Index>& Member() const noexcept
        {
            return GetMemberFromHolder<Index>(Members);
        }

        template <typename Function>
        constexpr void Apply(Function&& function) noexcept
        {
            ApplyImpl(Forward<Function&&>(function), MakeIndexSequence<Types::Count>{});
        }

        template <typename Function>
        constexpr void Apply(Function&& function) const noexcept
        {
            ApplyImpl(Forward<Function&&>(function), MakeIndexSequence<Types::Count>{});
        }

    private:
        template <typename Function, usize... Indices>
        constexpr void ApplyImpl(Function&& function, IndexSequence<Indices...>) noexcept
        {
            function(Member<Indices>()...);
        }

        template<usize TargetIndex, typename Node>
        static constexpr auto& GetMemberFromHolder(Node& node) noexcept
        {
            if constexpr (Node::Index() == TargetIndex)
                return node.Member;
            else
                return GetMemberFromHolder<TargetIndex>(node.NextMember);
        }

    private:
        HeadType Members;
    };

    template <typename... ArgTypes>
    MC_NODISCARD constexpr Tuple<ArgTypes&&...> ForwardAsTuple(ArgTypes&&... args) noexcept
    {
        return Tuple<ArgTypes&&...>(Forward<ArgTypes>(args)...);
    }
}