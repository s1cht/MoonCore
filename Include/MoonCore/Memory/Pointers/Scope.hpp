#pragma once
#include "Deleter.hpp"
#include "MoonCore/Concept.hpp"

namespace MC::Memory
{
    template <typename T, Concept::Deleter<T> D = DefaultDeleter<T>, typename BaseClass = void>
    class Scope
    {
    public:
        using Pointer = T*;
        using Reference = T&;
        using Base = Conditional<IsSame<BaseClass, void>, void, BaseClass*>;

    public:
        constexpr Scope() noexcept : m_Ptr(nullptr) {}
        constexpr Scope(NullptrT) noexcept : m_Ptr(nullptr) {}

        constexpr explicit Scope(T* ptr) noexcept : m_Ptr(ptr) {}

        constexpr Scope(Scope&& other) noexcept : m_Ptr(other.m_Ptr)
        {
            other.m_Ptr = nullptr;
        }

        template <typename Y, Concept::Deleter<Y> OD>
            requires Concept::ConvertibleTo<Y*, T*>
        constexpr Scope(Scope<Y, OD>&& other) noexcept 
            : m_Ptr(static_cast<T*>(other.Release())) 
        {}

        constexpr ~Scope() noexcept
        {
            if (m_Ptr) D::Delete(m_Ptr);
            m_Ptr = nullptr;
        }

        Scope(const Scope&) = delete;

    public:
        Scope& operator=(const Scope&) = delete;
        constexpr Scope& operator=(Scope&& other) noexcept
        {
            if (this != &other)
            {
                if (m_Ptr) D::Delete(m_Ptr);
                m_Ptr = other.m_Ptr;
                other.m_Ptr = nullptr;
            }
            return *this;
        }

    public:
        MC_NODISCARD_MSG("Possible dangling pointer, released pointer must be assigned and deleted properly.") 
        constexpr Pointer Release() noexcept
        {
            Pointer temp = m_Ptr;
            m_Ptr = nullptr;
            return temp;
        }

        constexpr void Reset(T* ptr = nullptr) noexcept
        {
            if (m_Ptr) D::Delete(m_Ptr);
            m_Ptr = ptr;
        }

        constexpr void Swap(Scope& other) noexcept
        {
            Pointer temp = other.m_Ptr;
            other.m_Ptr = m_Ptr;
            m_Ptr = temp;
        }

    public:
        template <typename Y>
            requires Concept::ConvertibleTo<Y*, T*>
        MC_NODISCARD constexpr Y* GetAs() const noexcept
        {
            return static_cast<Y*>(m_Ptr);
        }

        MC_NODISCARD constexpr void GetBase() const noexcept
        {
            return static_cast<Base>(m_Ptr);
        }

        MC_NODISCARD constexpr Pointer Get() const noexcept { return m_Ptr; }
        MC_NODISCARD constexpr const Pointer& GetDirect() const noexcept { return m_Ptr; }
        MC_NODISCARD constexpr Pointer& GetDirect() noexcept { return m_Ptr; }
        MC_NODISCARD constexpr Pointer operator->() const noexcept { return m_Ptr; }
        MC_NODISCARD constexpr Reference operator*() const noexcept { return *m_Ptr; }

        MC_NODISCARD constexpr bool IsValid() const noexcept { return m_Ptr != nullptr; }
        MC_NODISCARD constexpr operator bool() const noexcept { return m_Ptr != nullptr; }

        MC_NODISCARD constexpr bool operator==(const Scope& other) const noexcept
        {
            return m_Ptr == other.m_Ptr;
        }

        MC_NODISCARD constexpr bool operator==(NullptrT) const noexcept
        {
            return m_Ptr == nullptr;
        }

    private:
        Pointer m_Ptr;
    };

    template<typename T, Concept::Deleter<T> D = DefaultDeleter<T>, typename... ArgTypes>
        requires Concept::HasContructor<T, ArgTypes...>
    constexpr Scope<T, D> MakeScope(ArgTypes&&... arguments) noexcept
    {
        return Scope<T, D>(new(std::nothrow) T(Forward<ArgTypes>(arguments)...));
    }
}
