#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Clock/Time.hpp"
#include "MoonCore/Utility/Debug.hpp"

#include "WaitStrategy.hpp"
#include "Atomic.hpp"

namespace MC::Async
{
    template<u64 MaxValue>
        requires (MaxValue > 0)
    class Semaphore
    {
    public:
        constexpr explicit Semaphore(u64 desired) noexcept
            : m_Counter(desired)
        {}

        constexpr ~Semaphore() noexcept = default;

        Semaphore(const Semaphore&) = delete;
        Semaphore(Semaphore&&) = delete;
        Semaphore& operator=(const Semaphore&) = delete;
        Semaphore& operator=(Semaphore&&) = delete;

    public:
        constexpr static u64 Max() noexcept { return MaxValue; }

    public:
        void Release(u64 update = 1) noexcept
        {
            if (update == 0) return;
            u64 current = m_Counter.load(std::memory_order_relaxed);
            for (;;)
            {
                MC_CHECK(update <= MaxValue - current, "Semaphore overflow");
                if (m_Counter.compare_exchange_weak(current, current + update,
                    std::memory_order_release,
                    std::memory_order_relaxed))
                    break;
            }
            if (update == 1)
                WaitStrategy::Notify(&m_Counter);
            else
                WaitStrategy::NotifyAll(&m_Counter);
        }

        void Acquire() noexcept
        {
            for (;;)
            {
                u64 cur = m_Counter.load(std::memory_order_relaxed);
                if (cur > 0 && m_Counter.compare_exchange_weak(
                    cur, cur - 1,
                    std::memory_order_acquire,
                    std::memory_order_relaxed))
                    return;

                u64 zero = 0;
                WaitStrategy::Wait(&m_Counter, &zero, sizeof(u64), WAIT_INFINITE);
            }
        }

        bool TryAcquire() noexcept
        {
            u64 current = m_Counter.load(std::memory_order_relaxed);
            return current > 0 && m_Counter.compare_exchange_strong(
                current, current - 1,
                std::memory_order_acquire,
                std::memory_order_relaxed);
        }

        bool TryAcquireFor(Clock::Timestep duration) noexcept
        {
            auto deadline = Clock::Time::Now() + duration;
            for (;;) 
            {
                if (TryAcquire()) return true;
                Clock::Timestep remaining = deadline - Clock::Time::Now();
                if (remaining <= 0) return false;
                u64 zero = 0;
                if (!WaitStrategy::Wait(&m_Counter, &zero, sizeof(u64), static_cast<u32>(remaining.AsMilliseconds())))
                    return false;
            }
        }

        bool TryAcquireUntil(Clock::Timepoint timepoint) noexcept
        {
            Clock::Timepoint now = Clock::Time::Now();
            if (timepoint <= now) return TryAcquire();
            return TryAcquireFor(timepoint - now);
        }

    private:
        AtomicU64 m_Counter;
    };

    template<>
    class Semaphore<1>
    {
    public:
        constexpr explicit Semaphore(u64 desired) noexcept
            : m_Counter(static_cast<u8>(desired))
        {
            MC_CHECK(desired <= 1, "Desired > 1");
        }

        constexpr ~Semaphore() noexcept = default;

        Semaphore(const Semaphore&) = delete;
        Semaphore(Semaphore&&) = delete;
        Semaphore& operator=(const Semaphore&) = delete;
        Semaphore& operator=(Semaphore&&) = delete;

    public:
        constexpr static u64 Max() noexcept { return 1; }

    public:
        void Release(u64 update = 1) noexcept
        {
            if (update == 0) return;
            MC_CHECK(update <= 1, "Semaphore overflow");
            u8 expected = 0;
            MC_CHECK(m_Counter.compare_exchange_strong(expected, 1,
                std::memory_order_release,
                std::memory_order_relaxed), "Semaphore overflow");
            WaitStrategy::Notify(&m_Counter);
        }

        void Acquire() noexcept
        {
            for (;;)
            {
                u8 previous = m_Counter.exchange(0);
                if (previous == 1) break;
                u8 zero = 0;
                WaitStrategy::Wait(&m_Counter, &zero, sizeof(u8), WAIT_INFINITE);
            }
        }

        bool TryAcquire() noexcept
        {
            return m_Counter.exchange(0, std::memory_order_acquire) != 0;
        }

        bool TryAcquireFor(Clock::Timestep duration)
        {
            auto deadline = Clock::Time::Now() + duration;
            for (;;) 
            {
                if (TryAcquire()) return true;
                Clock::Timestep remaining = deadline - Clock::Time::Now();
                if (remaining <= 0) return false;
                u8 zero = 0;
                if (!WaitStrategy::Wait(&m_Counter, &zero, sizeof(u8), static_cast<u32>(remaining.AsMilliseconds())))
                    return false;
            }
        }

        bool TryAcquireUntil(Clock::Timepoint timepoint) noexcept
        {
            Clock::Timepoint now = Clock::Time::Now();
            if (timepoint <= now) return TryAcquire();
            return TryAcquireFor(timepoint - now);
        }

    private:
        AtomicU8 m_Counter;
    };

    using BinarySemaphore = Semaphore<1>;
}