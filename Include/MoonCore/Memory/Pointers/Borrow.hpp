#pragma once
#include "Scope.hpp"
#include "Reference.hpp"

namespace MC::Memory
{
    template <typename T, typename BaseClass = void>
    class Borrow
    {
    public:
        using Pointer = T*;
        using Reference = T&;
        using Base = Conditional<IsSame<BaseClass, void>, void, BaseClass*>;

    public:
        constexpr Borrow() noexcept : m_Ptr(nullptr) {}
        constexpr Borrow(NullptrT) noexcept : m_Ptr(nullptr) {}

        constexpr explicit Borrow(T* ptr) noexcept : m_Ptr(ptr) {}

        template <typename U>
            requires Concept::ConvertibleTo<U*, Pointer>
        constexpr Borrow(const Borrow<U>& ptr) noexcept : m_Ptr(ptr.Get()) {}

        template <typename U> 
            requires Concept::ConvertibleTo<U*, Pointer>
        constexpr Borrow(const Scope<U>& ptr) noexcept : m_Ptr(static_cast<Pointer>(ptr.Get())) {}

        template <typename U> 
            requires Concept::ConvertibleTo<U*, Pointer>
        constexpr Borrow(const Memory::Reference<U>& ptr) noexcept : m_Ptr(static_cast<Pointer>(ptr.Get())) {}

        template <typename Y>
            requires Concept::ConvertibleTo<Y*, Pointer>
        constexpr Borrow(Borrow<Y>&& other) noexcept 
            : m_Ptr(static_cast<Pointer>(other.m_Ptr))
        {
            other.m_Ptr = nullptr;
        }

        template <typename U> constexpr Borrow(Scope<U>&& other) noexcept = delete;
        template <typename U> constexpr Borrow(Memory::Reference<U>&& other) noexcept = delete;

        constexpr ~Borrow() noexcept { m_Ptr == nullptr; }

    public:
        template <typename U>
            requires Concept::ConvertibleTo<U*, Pointer>
        constexpr Borrow& operator=(const Borrow<U>& other)
        {
            m_Ptr = other.m_Ptr;
            return *this;
        }

        template <typename U>
            requires Concept::ConvertibleTo<U*, Pointer>
        constexpr Borrow& operator=(const Scope<U>& other)
        {
            m_Ptr = other.Get();
            return *this;
        }
        
        template <typename U>
            requires Concept::ConvertibleTo<U*, Pointer>
        constexpr Borrow& operator=(const Memory::Reference<U>& other)
        {
            m_Ptr = other.Get();
            return *this;
        }

        template <typename U>
            requires Concept::ConvertibleTo<U*, Pointer>
        constexpr Borrow& operator=(Borrow&& other) noexcept
        {
            if (this != &other)
            {
                m_Ptr = other.m_Ptr;
                other.m_Ptr = nullptr;
            }
            return *this;
        }

        template <typename U> constexpr Borrow& operator=(Scope<U>&& other) noexcept = delete;
        template <typename U> constexpr Borrow& operator=(Memory::Reference<U>&& other) noexcept = delete;

    public:
        constexpr Pointer Release() noexcept
        {
            Pointer temp = m_Ptr;
            m_Ptr = nullptr;
            return temp;
        }

        constexpr void Reset(T* ptr = nullptr) noexcept
        {
            m_Ptr = ptr;
        }

        constexpr void Swap(Borrow& other) noexcept
        {
            Pointer temp = other.m_Ptr;
            other.m_Ptr = m_Ptr;
            m_Ptr = temp;
        }

    public:
        template <typename Y>
            requires Concept::ConvertibleTo<Y*, T*>
        MOON_NODISCARD constexpr Y* GetAs() const noexcept
        {
            return static_cast<Y*>(m_Ptr);
        }

        MOON_NODISCARD constexpr void GetBase() const noexcept
        {
            return static_cast<Base>(m_Ptr);
        }

        MOON_NODISCARD constexpr Pointer Get() const noexcept { return m_Ptr; }
        MOON_NODISCARD constexpr const Pointer& GetDirect() const noexcept { return m_Ptr; }
        MOON_NODISCARD constexpr Pointer& GetDirect() noexcept { return m_Ptr; }
        MOON_NODISCARD constexpr Pointer operator->() const noexcept { return m_Ptr; }
        MOON_NODISCARD constexpr Reference operator*() const noexcept { return *m_Ptr; }

        MOON_NODISCARD constexpr bool IsValid() const noexcept { return m_Ptr != nullptr; }
        MOON_NODISCARD constexpr operator bool() const noexcept { return m_Ptr != nullptr; }

        MOON_NODISCARD constexpr bool operator==(const Borrow& other) const noexcept
        {
            return m_Ptr == other.m_Ptr;
        }

        MOON_NODISCARD constexpr bool operator==(NullptrT) const noexcept
        {
            return m_Ptr == nullptr;
        }

    private:
        Pointer m_Ptr;
    };
}