#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Memory/Pointers/Scope.hpp"

namespace MC::Async
{
    inline constexpr u32 WAIT_INFINITE = 0xFFFFFFFF;

    class WaitStrategy
    {
    public:
        virtual ~WaitStrategy() noexcept = default;
        virtual bool WaitImpl(volatile void* waitable, void* comparable, usize size, u32 timeout) noexcept = 0;
        virtual void NotifyImpl(void* waitable) noexcept = 0;
        virtual void NotifyAllImpl(void* waitable) noexcept = 0;

    public:
        static bool Wait(volatile void* waitable, void* comparable, usize size, u32 timeout)
        {
            return s_WaitStrategy->WaitImpl(waitable, comparable, size, timeout);
        }

        static void Notify(void* waitable) noexcept
        {
            s_WaitStrategy->NotifyImpl(waitable);
        }

        static void NotifyAll(void* waitable) noexcept
        {
            s_WaitStrategy->NotifyAllImpl(waitable);
        }

        static void Shutdown() noexcept
        {
            s_WaitStrategy.Reset();
        }

        static void Initialize() noexcept;

    private:
        static MC::Memory::Scope<WaitStrategy> s_WaitStrategy;
    };
}