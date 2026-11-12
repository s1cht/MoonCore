#pragma once
#include "MoonCore/Types.hpp"

#include "Debug.hpp"
#include "MoonCore/Memory/Memory.hpp"

namespace MC::Utility
{
    constexpr auto Null = nullptr;
    using NullType = decltype(Null);

    template<typename T>
    class Nullable
    {
    public:
        using CleanT = RemoveCV<T>;

    public:
        constexpr Nullable(T&& value) noexcept : m_Value(Move(value)), m_Exist(true) {}
        template<typename... ArgTypes>
        constexpr Nullable(ArgTypes... args) noexcept : m_Value(T(Forward<ArgTypes>(args)...)), m_Exist(true) {}
        template<typename U = CleanT>
        constexpr Nullable(U&& value) noexcept : m_Value(Forward<U>(value)), m_Exist(true) {}
        constexpr Nullable(NullType const) noexcept : m_Exist(false) {}

        constexpr ~Nullable() noexcept
        {
            if constexpr (!IsTriviallyDestructible<T>)
                Memory::Destroy(&m_Value);
        }

    public:
        constexpr operator bool() const noexcept { return m_Exist; }
        constexpr bool HasValue() const noexcept { return m_Exist; }

    public:
        constexpr T& Value() & noexcept
        {
            MC_CHECK(m_Exist, "Trying to access null object");
            return m_Value;
        }
        constexpr const T& Value() const& noexcept
        {
            MC_CHECK(m_Exist, "Trying to access null object");
            return m_Value;
        }
        constexpr T&& Value() && noexcept
        {
            MC_CHECK(m_Exist, "Trying to access null object");
            return Move(m_Value);
        }
        constexpr const T&& Value() const&& noexcept
        {
            MC_CHECK(m_Exist, "Trying to access null object");
            return Move(m_Value);
        }

        template <typename U = CleanT>
        constexpr CleanT ValueOr(U&& defaultValue) const& noexcept
        {
            if (m_Exist) return m_Value;
            return static_cast<CleanT>(Forward<U>(defaultValue));
        }

        template <typename U = CleanT>
        constexpr CleanT ValueOr(U&& defaultValue) && noexcept
        {
            if (m_Exist) return Move(m_Value);
            return static_cast<CleanT>(Forward<U>(defaultValue));
        }

        constexpr T* operator->() noexcept { return m_Exist ? &m_Value : nullptr; }
        constexpr const T* operator->() const noexcept { return m_Exist ? &m_Value : nullptr; }
        constexpr T& operator*() & noexcept { return m_Value; }
        constexpr const T& operator*() const& noexcept { return m_Value; }
        constexpr T&& operator*() && noexcept { return Move(m_Value); }
        constexpr const T&& operator*() const&& noexcept { return Move(m_Value); }

    public:
        template<typename F>
        constexpr auto AndThen(F&& function) & noexcept(IsNothrowInvocable<F&&, T&>)
        {
            if (*this)
                return Invoke(Forward<F>(function), Value());
            return RemoveCVReference<InvokeResult<F, T&>>{};
        }

        template<typename F>
        constexpr auto AndThen(F&& function) const& noexcept(IsNothrowInvocable<F&&, const T&>)
        {
            if (*this)
                return Invoke(Forward<F>(function), Value());
            return RemoveCVReference<InvokeResult<F, const T&>>{};
        }

        template<typename F>
        constexpr auto AndThen(F&& function) && noexcept(IsNothrowInvocable<F&&, T&&>)
        {
            if (*this)
                return Invoke(Forward<F>(function), Value());
            return RemoveCVReference<InvokeResult<F, T>>{};
        }

        template<typename F>
        constexpr auto AndThen(F&& function) const&& noexcept(IsNothrowInvocable<F&&, const T&&>)
        {
            if (*this)
                return Invoke(Forward<F>(function), Value());
            return RemoveCVReference<InvokeResult<F, const T>>{};
        }

        template<typename F>
        constexpr auto Transform(F&& function) & noexcept(IsNothrowInvocable<F&&, T&>)
            -> Nullable<RemoveCV<InvokeResult<F&&, T&>>>
        {
            if (*this)
                return Nullable<RemoveCV<InvokeResult<F&&, T&>>>(Invoke(Forward<F>(function), Value()));
            return Null;
        }

        template<typename F>
        constexpr auto Transform(F&& function) const& noexcept(IsNothrowInvocable<F&&, const T&>)
            -> Nullable<RemoveCV<InvokeResult<F&&, const T&>>>
        {
            if (*this)
                return Nullable<RemoveCV<InvokeResult<F&&, const T&>>>(Invoke(Forward<F>(function), Value()));
            return Null;
        }

        template<typename F>
        constexpr auto Transform(F&& function) && noexcept(IsNothrowInvocable<F&&, T>)
            -> Nullable<RemoveCV<InvokeResult<F&&, T>>>
        {
            if (*this)
                return Nullable<RemoveCV<InvokeResult<F&&, T>>>(Invoke(Forward<F>(function), Value()));
            return Null;
        }

        template<typename F>
        constexpr auto Transform(F&& function) const&& noexcept(IsNothrowInvocable<F&&, T&>)
            -> Nullable<RemoveCV<InvokeResult<F&&, const T>>>
        {
            if (*this)
                return Nullable<RemoveCV<InvokeResult<F&&, const T>>>(Invoke(Forward<F>(function), Move(Value())));
            return Null;
        }

        template<typename F>
        constexpr Nullable OrElse(F&& function) const& noexcept(IsNothrowInvocable<F&&, T&>)
        {
            return m_Exist ? *this : Forward<F>(function)();
        }

        template<typename F>
        constexpr Nullable OrElse(F&& function) && noexcept(IsNothrowInvocable<F&&, T&>)
        {
            return m_Exist ? Move(*this) : Forward<F>(function)();
        }

    private:
        T m_Value;
        bool m_Exist;
    };

    template<typename T>
    static constexpr Nullable<Decay<T>> MakeNullable(T&& value) noexcept
    {
        return Nullable<Decay<T>>(Forward<T>(value));
    }

    template<typename T, typename... ArgTypes>
    static constexpr Nullable<T> MakeNullable(ArgTypes... args) noexcept
    {
        return Nullable<T>(Forward<ArgTypes>(args)...);
    }
}