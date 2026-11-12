#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "Binder.hpp"

namespace MC::Callables
{
    namespace Helper
    {
        template<typename ReturnType, typename... ArgTypes>
        class Invokable
        {
        public:
            virtual ~Invokable() = default;
            virtual ReturnType Invoke(ArgTypes&&... arguments) const = 0;
            MC_NODISCARD virtual Invokable* Clone() const noexcept = 0;
        };

        template<typename Signature, typename ReturnType, typename... ArgTypes>
        class InvokableImpl : public Invokable<ReturnType, ArgTypes...>
        {
        public:
            using Base = Invokable<ReturnType, ArgTypes...>;

        public:
            explicit InvokableImpl(Signature function) : m_Function(Move(function)) {}

            ReturnType Invoke(ArgTypes&&... arguments) const override
            {
                return MC::Invoke(m_Function, Forward<ArgTypes>(arguments)...);
            }

            MC_NODISCARD Base* Clone() const noexcept override
            {
                return new(std::nothrow) InvokableImpl(m_Function);
            }

        private:
            mutable Signature m_Function;
        };
    }

    template <typename Signature>
    class Callable;

    template<typename ReturnType, typename... ArgTypes>
    class Callable<ReturnType(ArgTypes...)>
    {
        using Storage = Helper::Invokable<ReturnType, ArgTypes...>;
        template <typename Function>
        using Implementation = Helper::InvokableImpl<Function, ReturnType, ArgTypes...>;

        static constexpr usize SBOSize = 32;
        static constexpr usize SBOAlignment = alignof(MaxAlignT);

    public:
        constexpr Callable() noexcept 
            : m_Ptr(nullptr), m_UsingSBO(false) 
        {}

        constexpr Callable(nullptr_t) noexcept
            : m_Ptr(nullptr), m_UsingSBO(false)
        {}

        template<typename Function>
        Callable(Function&& function) noexcept 
            : m_UsingSBO(false)
        {
            Store(Move(function));
        }

        Callable(const Callable& other) noexcept    
            : m_UsingSBO(other.m_UsingSBO)
        {
            if (m_UsingSBO)
            {
                memcpy(m_Buffer, other.m_Buffer, SBOSize);
                m_Ptr = reinterpret_cast<Storage*>(&m_Buffer);
            }
            else
            {
                m_Ptr = other.m_Ptr ? other.m_Ptr->Clone() : nullptr;
            }
        }
        
        Callable(Callable&& other) noexcept 
            : m_UsingSBO(other.m_UsingSBO)
        {
            if (m_UsingSBO)
            {
                memmove(m_Buffer, other.m_Buffer, SBOSize);
                m_Ptr = reinterpret_cast<Storage*>(&m_Buffer);
                other.m_Ptr = nullptr;
            }
            else
            {
                m_Ptr = other.m_Ptr;
                other.m_Ptr = nullptr;
            }
            other.m_UsingSBO = false;
        }

        ~Callable() noexcept
        {
            Destroy();
        }

    public:
        Callable& operator=(const Callable& other) noexcept
        {
            if (this != &other)
            {
                Destroy();
                m_UsingSBO = false;
                m_Ptr = other.m_Ptr ? other.m_Ptr->Clone() : nullptr;
            }
            return *this;
        }

        Callable& operator=(Callable&& other) noexcept
        {
            if (this != &other)
            {
                Destroy();
                m_UsingSBO = other.m_UsingSBO;

                if (m_UsingSBO)
                {
                    memmove(m_Buffer, other.m_Buffer, SBOSize);
                    m_Ptr = reinterpret_cast<Storage*>(&m_Buffer);
                }
                else
                {
                    m_Ptr = other.m_Ptr;
                }
                other.m_Ptr = nullptr;
                other.m_UsingSBO = false;
            }
            return *this;
        }

    public:
        ReturnType operator()(ArgTypes... arguments) const 
        {
            MC_CHECK(m_Ptr, "Can't call a function that is a nullptr");
            return m_Ptr->Invoke(Forward<ArgTypes>(arguments)...);
        }

        inline explicit operator bool() const noexcept { return IsValid(); }
        inline bool IsValid() const noexcept { return m_Ptr != nullptr; }

    private:
        template <typename Function>
        void Store(Function&& function) noexcept
        {
            using Impl = Implementation<Decay<Function>>;
            if constexpr (sizeof(Impl) <= SBOSize
                && alignof(Impl) <= SBOAlignment
                && IsNothrowMoveConstructible<Function>)
            {
                new (&m_Buffer) Impl(Forward<Function>(function));
                m_Ptr = reinterpret_cast<Storage*>(&m_Buffer);
                m_UsingSBO = true;
            }
            else
            {
                m_Ptr = new(std::nothrow) Impl(Forward<Function>(function));
                m_UsingSBO = false;
            }
        }

        void Destroy() noexcept
        {
            if (!m_Ptr) return;
            if (m_UsingSBO)
                m_Ptr->~Storage();
            else
                delete m_Ptr;
            m_Ptr = nullptr;
        }

    private:
        Storage* m_Ptr;
        alignas(SBOAlignment) u8 m_Buffer[SBOSize];
        bool m_UsingSBO = false;
    };

    template<typename ReturnType, typename... ArgTypes>
    class Callable<ReturnType(*)(ArgTypes...)>
    {
        using FunctionPtr = ReturnType(*)(ArgTypes...);

    public:
        Callable(FunctionPtr func) noexcept : m_Ptr(func) {}

        Callable() noexcept : m_Ptr(nullptr) {}

        Callable(const Callable&) noexcept = default;
        Callable(Callable&&) noexcept = default;
        Callable& operator=(const Callable&) noexcept = default;
        Callable& operator=(Callable&&) noexcept = default;

        ~Callable() = default;

        ReturnType operator()(ArgTypes... arguments) const
        {
            MC_CHECK(m_Ptr, "Can't call a function that is a nullptr");
            return m_Ptr(Forward<ArgTypes>(arguments)...);
        }

        bool IsValid() const noexcept { return m_Ptr != nullptr; }
        explicit operator bool() const noexcept { return IsValid(); }

    private:
        FunctionPtr m_Ptr;
    };

    template<typename ReturnType, typename... ArgTypes>
    Callable(ReturnType(*)(ArgTypes...)) -> Callable<ReturnType(*)(ArgTypes...)>;
}
