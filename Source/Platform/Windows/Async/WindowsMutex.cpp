#include "WindowsMutex.hpp"

namespace MC::Windows::Async
{
    WindowsMutex::WindowsMutex() noexcept
        : m_Mutex(), m_OwningThread(0)
    {
        InitializeCriticalSection(&m_Mutex);
    }

    void WindowsMutex::Lock() const noexcept
    {
        EnterCriticalSection(&m_Mutex);
        m_OwningThread.store(GetCurrentThreadId(), std::memory_order_relaxed);
    }

    bool WindowsMutex::TryLock() const noexcept
    {
        if (TryEnterCriticalSection(&m_Mutex))
        {
            m_OwningThread.store(GetCurrentThreadId(), std::memory_order_relaxed);
            return true;
        }
        return false;
    }

    bool WindowsMutex::TryLockFor(Clock::Timestep duration) const noexcept
    {
        const Clock::Timepoint end = Clock::Time::Now() + duration;
        while (Clock::Time::Now() <= end)
            if (TryLock())
                return true;
        return false;
    }

    bool WindowsMutex::TryLockUntil(Clock::Timepoint timepoint) const noexcept
    {
        const Clock::Timepoint now = Clock::Time::Now();
        if (timepoint <= now) return TryLock();
        return TryLockFor(timepoint - now);
    }

    void WindowsMutex::Unlock() const noexcept
    {
        m_OwningThread.store(0, std::memory_order_relaxed);
        LeaveCriticalSection(&m_Mutex);
    }

    bool WindowsMutex::ThreadOwnsExclusively() const noexcept
    {
        return m_OwningThread.load() == GetCurrentThreadId();
    }

    WindowsSharedMutex::WindowsSharedMutex() noexcept
        : m_OwningThread(0)
    {
        InitializeSRWLock(&m_Mutex);
    }

    void WindowsSharedMutex::Lock() const noexcept
    {
        AcquireSRWLockExclusive(&m_Mutex);
        m_OwningThread.store(GetCurrentThreadId(), std::memory_order_relaxed);
    }

    bool WindowsSharedMutex::TryLock() const noexcept
    {
        if (TryAcquireSRWLockExclusive(&m_Mutex))
        {
            m_OwningThread.store(GetCurrentThreadId(), std::memory_order_relaxed);
            return true;
        }
        return false;
    }

    bool WindowsSharedMutex::TryLockFor(Clock::Timestep duration) const noexcept
    {
        const Clock::Timepoint end = Clock::Time::Now() + duration;
        while (Clock::Time::Now() <= end)
            if (TryLock()) 
                return true;
        return false;
    }

    bool WindowsSharedMutex::TryLockUntil(Clock::Timepoint timepoint) const noexcept
    {
        const Clock::Timepoint now = Clock::Time::Now();
        if (timepoint <= now) return TryLock();
        return TryLockFor(timepoint - now);
    }

    void WindowsSharedMutex::Unlock() const noexcept
    {
        m_OwningThread.store(0, std::memory_order_relaxed);
        ReleaseSRWLockExclusive(&m_Mutex);
    }

    void WindowsSharedMutex::LockShared() const noexcept
    {
        AcquireSRWLockShared(&m_Mutex);
    }

    bool WindowsSharedMutex::TryLockShared() const noexcept
    {
        return TryAcquireSRWLockShared(&m_Mutex);
    }

    bool WindowsSharedMutex::TryLockForShared(Clock::Timestep duration) const noexcept
    {
        const Clock::Timepoint end = Clock::Time::Now() + duration;
        while (Clock::Time::Now() <= end)
            if (TryLockShared())
                return true;
        return false;
    }

    bool WindowsSharedMutex::TryLockUntilShared(Clock::Timepoint timepoint) const noexcept
    {
        const Clock::Timepoint now = Clock::Time::Now();
        if (timepoint <= now) return TryLockShared();
        return TryLockForShared(timepoint - now);
    }

    void WindowsSharedMutex::UnlockShared() const noexcept
    {
        ReleaseSRWLockShared(&m_Mutex);
    }

    bool WindowsSharedMutex::ThreadOwnsExclusively() const noexcept
    {
        return m_OwningThread.load() == GetCurrentThreadId();
    }
}

MC::Memory::Scope<MC::Async::MutexBase> MC::Async::MutexBase::Create()
{
    return Memory::MakeScope<MC::Windows::Async::WindowsMutex>();
}

MC::Memory::Scope<MC::Async::SharedMutexBase> MC::Async::SharedMutexBase::Create()
{
    return Memory::MakeScope<MC::Windows::Async::WindowsSharedMutex>();
}