#pragma once
#include "Atomic.hpp"

namespace MC::Async
{
    class TimelineSemaphore
    {
    public:
        constexpr TimelineSemaphore() noexcept : m_Value(0) {}
        explicit constexpr TimelineSemaphore(u64 initial) noexcept : m_Value(initial) {}
        constexpr ~TimelineSemaphore() noexcept = default;
        
        TimelineSemaphore(const TimelineSemaphore&) = delete;
        TimelineSemaphore(TimelineSemaphore&&) noexcept = delete;
        
    public:
        TimelineSemaphore& operator=(const TimelineSemaphore&) = delete;
        TimelineSemaphore& operator=(TimelineSemaphore&&) noexcept = delete;

    public:
        void Signal(u64 desired) noexcept
        {
            u64 current = m_Value.load(std::memory_order_relaxed);
            while (current < desired &&
                !m_Value.compare_exchange_weak(current, desired,
                    std::memory_order_release, std::memory_order_relaxed)) {
            }
            if (current < desired)
                m_Value.notify_all();
        }
        u64 Value() const noexcept { return m_Value.load(std::memory_order_acquire); }
        void Reset() noexcept
        {
            m_Value.store(0, std::memory_order_relaxed);
        }
        void WaitFor(u64 value) const noexcept
        {
            u64 current = m_Value.load(std::memory_order_acquire);
            while (current < value)
            {
                m_Value.wait(current, std::memory_order_relaxed);
                current = m_Value.load(std::memory_order_acquire);
            }
        }

    private:
        AtomicU64 m_Value;
    };
}