#pragma once
#include "MoonCore/Async/Mutex.hpp"
#include "Platform/Windows/WindowsInclude.hpp"

namespace MC::Windows::Async
{
    class WindowsMutex final : public MC::Async::MutexBase
    {
    public:
        WindowsMutex() noexcept;
        WindowsMutex(const WindowsMutex&) = delete;

        ~WindowsMutex() noexcept override = default;

    public:
        WindowsMutex& operator=(const WindowsMutex&) = delete;

    public:
        void Lock() const noexcept override;
        bool TryLock() const noexcept override;
        bool TryLockFor(Clock::Timestep duration) const noexcept override;
        bool TryLockUntil(Clock::Timepoint timepoint) const noexcept override;
        void Unlock() const noexcept override;

        bool ThreadOwnsExclusively() const noexcept override;
    
    public:
        PCRITICAL_SECTION Handle() const noexcept { return &m_Mutex; }

    private:
        mutable CRITICAL_SECTION m_Mutex;
        mutable MC::Async::Atomic<DWORD> m_OwningThread;
    };

    class WindowsSharedMutex final : public MC::Async::SharedMutexBase
    {
    public:
        WindowsSharedMutex() noexcept;
        WindowsSharedMutex(const WindowsSharedMutex&) = delete;
        WindowsSharedMutex(WindowsSharedMutex&&) noexcept = delete;

        ~WindowsSharedMutex() noexcept override = default;

    public:
        WindowsSharedMutex& operator=(const WindowsSharedMutex&) = delete;
        WindowsSharedMutex& operator=(WindowsSharedMutex&&) noexcept = delete;

    public:
        void Lock() const noexcept override;
        bool TryLock() const noexcept override;
        bool TryLockFor(Clock::Timestep duration) const noexcept override;
        bool TryLockUntil(Clock::Timepoint timepoint) const noexcept override;
        void Unlock() const noexcept override;
        
        void LockShared() const noexcept override;
        bool TryLockShared() const noexcept override;
        bool TryLockForShared(Clock::Timestep duration) const noexcept override;
        bool TryLockUntilShared(Clock::Timepoint timepoint) const noexcept override;
        void UnlockShared() const noexcept override;

        bool ThreadOwnsExclusively() const noexcept override;

    public:
        PSRWLOCK Handle() const noexcept { return &m_Mutex; }

    private:
        mutable SRWLOCK m_Mutex;
        mutable MC::Async::Atomic<DWORD> m_OwningThread;
    };
}