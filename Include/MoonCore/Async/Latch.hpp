#pragma once

#include "MoonCore/Core.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "Atomic.hpp"

namespace MC::Async
{
    class Latch
    {
    public:
        static constexpr u64 Max() noexcept { return NumericLimit<u64>::Max; }

    public:
        explicit Latch(const u64 expected) noexcept : m_Counter(expected){}
        Latch(const Latch&) = delete;

    public:
        Latch& operator=(const Latch&) = delete;

    public:
        void CountDown(u64 n = 1) noexcept
        {
            MC_VALIDATE(n <= m_Counter.load(std::memory_order_acquire), "Latch: n > current counter!");
            u64 current = m_Counter.fetch_sub(n) - n;
        
            if (current == 0)
                m_Counter.notify_all();
        }
        
        bool TryWait() const noexcept
        {
            return m_Counter.load(std::memory_order_acquire) == 0;
        }

        void Wait() const noexcept
        {
            for (;;)
            {
                const u64 current = m_Counter.load(std::memory_order_acquire);
                if (current == 0) break;
                m_Counter.wait(current, std::memory_order_relaxed);
            }
        }

        void ArriveAndWait(u64 n = 1) noexcept
        {
            CountDown(n);
            Wait();
        }

    private:
        AtomicU64 m_Counter;
    };
}