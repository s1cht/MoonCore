#pragma once
#include "MoonCore/Core.hpp"
#include "Thread.hpp"
#include "MoonCore/Clock/Time.hpp"
#include "MoonCore/Containers/String/StringView.hpp"
#include "MoonCore/Memory/Pointers/Scope.hpp"

namespace MC::Concept
{
    template <class T>
    concept Lockable = IsNothrowDefaultConstructible<T> && 
        IsNothrowMoveConstructible<T> && requires(T mutex)
    {
        { mutex.Lock() };
        { mutex.Unlock() };
    };

    template <class T>
    concept MutexType = Lockable<T> && requires(T mutex)
    {
        { mutex.TryLock() } -> ConvertibleTo<bool>;
    };

    template <class T>
    concept SharedMutexType = MutexType<T> && requires(T mutex)
    {
        { mutex.LockShared() };
        { mutex.UnlockShared() };
        { mutex.TryLockShared() } -> ConvertibleTo<bool>;
    };

    template <class T>
    concept TimedType = requires(T mutex, Clock::Timestep duration, Clock::Timepoint timepoint)
    {
        { mutex.TryLockFor(duration) } -> ConvertibleTo<bool>;
        { mutex.TryLockUntil(timepoint) } -> ConvertibleTo<bool>;
    };

    template <class T>
    concept SharedTimedType = TimedType<T> && requires(T mutex, Clock::Timestep duration, Clock::Timepoint timepoint)
    {
        { mutex.TryLockSharedFor(duration) } -> ConvertibleTo<bool>;
        { mutex.TryLockSharedUntil(timepoint) } -> ConvertibleTo<bool>;
    };

    template <class T> concept TimedMutexType = MutexType<T> && TimedType<T>;
    template <class T> concept TimedSharedMutexType = SharedMutexType<T> && SharedTimedType<T>;

    template <MutexType T> constexpr bool MutexIsShared = SharedMutexType<T>;
    template <TimedMutexType T> constexpr bool TimedMutexIsShared = TimedSharedMutexType<T>;
}

namespace MC::Async
{
    namespace Tags
    {
        struct AdoptLock     { explicit AdoptLock() = default; };
        struct TryToLock     { explicit TryToLock() = default; };
        struct DeferredLock  { explicit DeferredLock() = default; };
    }

    class MutexBase
    {
    public:
        virtual ~MutexBase() noexcept = default;
        
        virtual void Lock() const noexcept = 0;
        virtual bool TryLock() const noexcept = 0;
        virtual bool TryLockFor(Clock::Timestep duration) const noexcept = 0;
        virtual bool TryLockUntil(Clock::Timepoint timepoint) const noexcept = 0;
        virtual void Unlock() const noexcept = 0;

        virtual bool ThreadOwnsExclusively() const noexcept = 0;

    public:
        static Memory::Scope<MutexBase> Create();
    };

    class SharedMutexBase : public MutexBase
    {
    public:
        virtual void LockShared() const noexcept = 0;
        virtual bool TryLockShared() const noexcept = 0;
        virtual bool TryLockForShared(Clock::Timestep duration) const noexcept = 0;
        virtual bool TryLockUntilShared(Clock::Timepoint timepoint) const noexcept = 0;
        virtual void UnlockShared() const noexcept = 0;

    public:
        static Memory::Scope<SharedMutexBase> Create();
    };

#if !defined(PLATFORM_WINDOWS) && !defined(PLATFORM_LINUX) && !defined(PLATFORM_MACOS)
    Memory::Scope<MutexBase> MutexBase::Create()
    {
        return nullptr;
    }

    Memory::Scope<SharedMutexBase> SharedMutexBase::Create()
    {
        return nullptr;
    }
#endif

    class Mutex
    {
    public:
        Mutex() noexcept : m_Mutex(MutexBase::Create()) {}
        Mutex(const Mutex&) = delete;
        Mutex(Mutex&&) noexcept = default;
        ~Mutex() noexcept = default;

    public:
        Mutex& operator=(const Mutex&) = delete;
        Mutex& operator=(Mutex&&) noexcept = default;

    public:
        void Lock() const noexcept { m_Mutex->Lock(); }
        bool TryLock() const noexcept { return m_Mutex->TryLock(); }
        bool TryLockFor(Clock::Timestep duration) const noexcept { return m_Mutex->TryLockFor(duration); }
        bool TryLockUntil(Clock::Timepoint timepoint) const noexcept { return m_Mutex->TryLockUntil(timepoint); }
        void Unlock() const noexcept { m_Mutex->Unlock(); }

        MutexBase* Handle() const noexcept { return m_Mutex.Get(); }

    private:
        Memory::Scope<MutexBase> m_Mutex;
    };

    class SharedMutex
    {
    public:
        SharedMutex() noexcept : m_Mutex(SharedMutexBase::Create()) {}
        SharedMutex(const SharedMutex&) = delete;
        SharedMutex(SharedMutex&&) noexcept = default;
        ~SharedMutex() noexcept = default;

    public:
        SharedMutex& operator=(const SharedMutex&) = delete;
        SharedMutex& operator=(SharedMutex&&) noexcept = default;

    public:
        void Lock() const noexcept { m_Mutex->Lock(); }
        bool TryLock() const noexcept { return m_Mutex->TryLock(); }
        bool TryLockFor(Clock::Timestep duration) const noexcept { return m_Mutex->TryLockFor(duration); }
        bool TryLockUntil(Clock::Timepoint timepoint) const noexcept { return m_Mutex->TryLockUntil(timepoint); }
        void Unlock() const noexcept { m_Mutex->Unlock(); }

        void LockShared() const noexcept { m_Mutex->LockShared(); }
        bool TryLockShared() const noexcept { return m_Mutex->TryLockShared(); }
        void UnlockShared() const noexcept { m_Mutex->UnlockShared(); }

        SharedMutexBase* Handle() const noexcept { return m_Mutex.Get(); }

    private:
        Memory::Scope<SharedMutexBase> m_Mutex;
    };

    class RecursiveMutex
    {
    public:
        RecursiveMutex() noexcept : m_Mutex(MutexBase::Create()), m_Depth(0) {}
        RecursiveMutex(const RecursiveMutex&) = delete;
        RecursiveMutex(RecursiveMutex&&) noexcept = default;
        ~RecursiveMutex() noexcept = default;

    public:
        RecursiveMutex& operator=(const RecursiveMutex&) = delete;
        RecursiveMutex& operator=(RecursiveMutex&&) noexcept = default;

    public:
        void Lock() const noexcept
        {
            if (!m_Mutex->ThreadOwnsExclusively())
                m_Mutex->Lock();
            ++m_Depth;
        }

        bool TryLock() const noexcept
        {
            if (m_Mutex->ThreadOwnsExclusively())
            {
                ++m_Depth;
                return true;
            }
            if (m_Mutex->TryLock())
            {
                ++m_Depth;
                return true;
            }
            return false;
        }

        bool TryLockFor(Clock::Timestep duration) const noexcept
        {
            if (m_Mutex->ThreadOwnsExclusively())
            {
                ++m_Depth;
                return true;
            }
            if (m_Mutex->TryLockFor(duration))
            {
                ++m_Depth;
                return true;
            }
            return false;
        }

        bool TryLockUntil(Clock::Timepoint timepoint) const noexcept
        {
            if (m_Mutex->ThreadOwnsExclusively())
            {
                ++m_Depth;
                return true;
            }
            if (m_Mutex->TryLockUntil(timepoint))
            {
                ++m_Depth;
                return true;
            }
            return false;
        }

        void Unlock() const noexcept
        {
            MC_CHECK(m_Mutex->ThreadOwnsExclusively(), "Unlock() from a thread that doesn't own the mutex");
            if (--m_Depth == 0)
                m_Mutex->Unlock();
        }

    private:
        Memory::Scope<MutexBase> m_Mutex;
        mutable u32 m_Depth;
    };

    class RecursiveSharedMutex
    {
    public:
        RecursiveSharedMutex() noexcept : m_Mutex(SharedMutexBase::Create()), m_Depth(0) {}
        RecursiveSharedMutex(const RecursiveSharedMutex&) = delete;
        RecursiveSharedMutex(RecursiveSharedMutex&&) noexcept = default;
        ~RecursiveSharedMutex() noexcept = default;

    public:
        RecursiveSharedMutex& operator=(const RecursiveSharedMutex&) = delete;
        RecursiveSharedMutex& operator=(RecursiveSharedMutex&&) noexcept = default;

    public:
        void Lock() const noexcept
        {
            if (!m_Mutex->ThreadOwnsExclusively())
                m_Mutex->Lock();
            ++m_Depth;
        }

        bool TryLock() const noexcept
        {
            if (m_Mutex->ThreadOwnsExclusively())
            {
                ++m_Depth;
                return true;
            }
            if (m_Mutex->TryLock())
            {
                ++m_Depth;
                return true;
            }
            return false;
        }

        bool TryLockFor(Clock::Timestep duration) const noexcept
        {
            if (m_Mutex->ThreadOwnsExclusively())
            {
                ++m_Depth;
                return true;
            }
            if (m_Mutex->TryLockFor(duration))
            {
                ++m_Depth;
                return true;
            }
            return false;
        }

        bool TryLockUntil(Clock::Timepoint timepoint) const noexcept
        {
            if (m_Mutex->ThreadOwnsExclusively())
            {
                ++m_Depth;
                return true;
            }
            if (m_Mutex->TryLockUntil(timepoint))
            {
                ++m_Depth;
                return true;
            }
            return false;
        }

        void Unlock() const noexcept
        {
            MC_CHECK(m_Mutex->ThreadOwnsExclusively(), "Unlock() from a thread that doesn't own the mutex");
            if (--m_Depth == 0)
                m_Mutex->Unlock();
        }

        void LockShared() const noexcept { m_Mutex->LockShared(); }
        bool TryLockShared() const noexcept { return m_Mutex->TryLockShared(); }
        void UnlockShared() const noexcept { m_Mutex->UnlockShared(); }

    private:
        Memory::Scope<SharedMutexBase> m_Mutex;
        mutable u32 m_Depth;
    };

    namespace Helper
    {
        template<Concept::MutexType Lockable1, Concept::MutexType Lockable2>
        static bool AttemptLock(Lockable1& lockable1, Lockable2& lockable2) noexcept
        {
            if (lockable1.TryLock())
            {
                if (lockable2.TryLock())
                    return false;
                lockable1.Unlock();
            }
            CurrentThread::Yield();
            return true;
        }

        template<Concept::MutexType Lockable, Concept::MutexType... Lockables>
        static i32 AttemptLockRecursive(const i32 index, Lockable& lockable, Lockables&... lockables) noexcept
        {
            if (lockable.TryLock())
            {
                if constexpr (sizeof...(Lockables)) return -1;
                i32 result = AttemptLockRecursive(index + 1, lockables);
                if (result != -1) lockable.Unlock();
                return result;
            }
            return index;
        }

        template<Concept::MutexType... Lockables>
        static i32 BeginAttemptingLock(Lockables&... lockables) noexcept
        {
            i32 result = AttemptLockRecursive(0, lockables);
            CurrentThread::Yield();
            return result;
        }
    }

    template<Concept::MutexType Lockable1, Concept::MutexType Lockable2, Concept::MutexType... Lockables>
    void Lock(Lockable1& lockable1, Lockable2& lockable2, MC_MAYBE_UNUSED Lockables&... lockables) noexcept
    {
        if constexpr (sizeof... (Lockables) == 0)
        {
            while (Helper::AttemptLock(lockable1, lockable2) && Helper::AttemptLock(lockable2, lockable1)) {}
        }
        else
        {
            i32 result = 0;
            while (result != -1) result = Helper::BeginAttemptingLock(lockable1, lockable2, lockables);
        }
    }

    template<Concept::Lockable MutexType>
    class LockGuard
    {
    public:
        explicit LockGuard(MutexType& mutex) noexcept : m_Mutex(&mutex) { m_Mutex->Lock(); }
        LockGuard(const LockGuard&) = delete;
        LockGuard(LockGuard&&) = delete;
        ~LockGuard() noexcept { m_Mutex->Unlock(); }

    private:
        MutexType* m_Mutex;
    };

    template <Concept::MutexType MutexType>
    class UniqueLock
    {
    public:
        UniqueLock() noexcept : m_Mutex(nullptr), m_Locked(false) {}
        UniqueLock(UniqueLock&&) noexcept = default;
        explicit UniqueLock(MutexType& mutex) noexcept : m_Mutex(mutex) { m_Mutex.Lock(); m_Locked.store(true); }
        UniqueLock(MutexType& mutex, Tags::AdoptLock) noexcept : m_Mutex(mutex), m_Locked(true) {}
        UniqueLock(MutexType& mutex, Tags::TryToLock) noexcept : m_Mutex(mutex), m_Locked(m_Mutex.TryLock()) {}
        UniqueLock(MutexType& mutex, Tags::DeferredLock) noexcept : m_Mutex(mutex) { m_Mutex.Lock(); m_Locked.store(true); }
        UniqueLock(MutexType& mutex, Clock::Timestep duration) noexcept : m_Mutex(mutex), m_Locked(m_Mutex.TryLockFor(duration)) {}
        UniqueLock(MutexType& mutex, Clock::Timepoint timepoint) noexcept : m_Mutex(mutex), m_Locked(m_Mutex.TryLockUntil(timepoint)) {}

        ~UniqueLock() noexcept
        {
            if (m_Locked.load())
                m_Mutex.Unlock();
        };

    public:
        operator bool() const noexcept { return m_Locked.load(); }

    public:
        MutexType* Mutex() const noexcept { return &m_Mutex; }
        bool OwnsLock() const noexcept { return m_Locked.load(); }

    public:
        void Lock() noexcept { m_Mutex.Lock(); m_Locked.store(true); }
        bool TryLock() noexcept { bool result = m_Mutex.TryLock(); m_Locked.store(result); return result; }
        bool TryLockFor(Clock::Timestep duration) noexcept
        {
            static_assert(Concept::TimedMutexType<MutexType>, "MutexType doesn't validate TimedMutexType concept in TryLockFor");
            bool result = m_Mutex.TryLockFor(duration);
            m_Locked.store(result);
            return result;
        }
        bool TryLockUntil(Clock::Timepoint timepoint) noexcept
        {
            static_assert(Concept::TimedMutexType<MutexType>, "MutexType doesn't validate TimedMutexType concept in TryLockUntil");
            bool result = m_Mutex.TryLockUntil(timepoint);
            m_Locked.store(result);
            return result;
        }
        void Unlock() noexcept { m_Mutex.Unlock(); m_Locked.store(false); }

        MutexType* Release() noexcept
        {
            MutexType* mutex = &m_Mutex;
            m_Mutex = nullptr;
            m_Locked.store(false);
            return mutex;
        }

        template<Concept::SharedMutexType Mutex>
        void Swap(UniqueLock<Mutex>& other) noexcept
        {
            bool tempLocked = m_Locked.exchange(other.m_Locked.load());
            other.m_Locked.store(tempLocked);
            MutexType& mutex = m_Mutex;
            m_Mutex = other.m_Mutex;
            other.m_Mutex = mutex;
        }

    private:
        MutexType& m_Mutex;
        AtomicBool m_Locked;
    };

    template <Concept::SharedMutexType MutexType>
    class SharedLock
    {
    public:
        SharedLock() noexcept : m_Mutex(nullptr), m_Locked(false) {}
        SharedLock(SharedLock&&) noexcept = default;
        explicit SharedLock(MutexType& mutex) noexcept : m_Mutex(mutex) { m_Mutex.LockShared(); m_Locked.store(true); }
        SharedLock(MutexType& mutex, Tags::AdoptLock) noexcept : m_Mutex(mutex), m_Locked(true) {}
        SharedLock(MutexType& mutex, Tags::TryToLock) noexcept : m_Mutex(mutex), m_Locked(m_Mutex.TryLockShared()) {}
        SharedLock(MutexType& mutex, Tags::DeferredLock) noexcept : m_Mutex(mutex) { m_Mutex.LockShared(); m_Locked.store(true); }
        SharedLock(MutexType& mutex, Clock::Timestep duration) noexcept
            : m_Mutex(mutex), m_Locked(m_Mutex.TryLockForShared(duration))
        {
            static_assert(Concept::TimedSharedMutexType<MutexType>, "MutexType doesn't validate TimedSharedMutexType concept in TryLockFor constructor");
        }
        SharedLock(MutexType& mutex, Clock::Timepoint timepoint) noexcept
            : m_Mutex(mutex), m_Locked(m_Mutex.TryLockUntilShared(timepoint))
        {
            static_assert(Concept::TimedSharedMutexType<MutexType>, "MutexType doesn't validate TimedSharedMutexType concept in TryLockUntil constructor");
        }

        ~SharedLock() noexcept
        {
            if (m_Locked.load())
                m_Mutex.UnlockShared();
        }

    public:
        operator bool() const noexcept
        {
            return m_Locked.load();
        }

    public:
        MutexType* Mutex() const noexcept { return &m_Mutex; }
        bool OwnsLock() const noexcept { return m_Locked.load(); }

    public:
        void Lock() noexcept { m_Mutex.LockShared(); m_Locked.store(true); }
        bool TryLock() noexcept { bool result = m_Mutex.TryLockShared(); m_Locked.store(result); return result; }
        bool TryLockFor(Clock::Timestep duration) noexcept
        {
            static_assert(Concept::TimedSharedMutexType<MutexType>, "MutexType doesn't validate TimedSharedMutexType concept in TryLockFor");
            bool result = m_Mutex.TryLockForShared(duration);
            m_Locked.store(result);
            return result;
        }
        bool TryLockUntil(Clock::Timepoint timepoint) noexcept
        {
            static_assert(Concept::TimedSharedMutexType<MutexType>, "MutexType doesn't validate TimedSharedMutexType concept in TryLockUntil");
            bool result = m_Mutex.TryLockUntilShared(timepoint);
            m_Locked.store(result);
            return result;
        }
        void Unlock() noexcept { m_Mutex.UnlockShared()(); m_Locked.store(false); }

        MutexType* Release() noexcept
        {
            MutexType* mutex = &m_Mutex;
            m_Mutex = nullptr;
            m_Locked.store(false);
            return mutex;
        }

        template<Concept::SharedMutexType Mutex>
        void Swap(SharedLock<Mutex>& other) noexcept
        {
            bool tempLocked = m_Locked.exchange(other.m_Locked.load());
            other.m_Locked.store(tempLocked);
            MutexType& mutex = m_Mutex;
            m_Mutex = other.m_Mutex;
            other.m_Mutex = mutex;
        }

    private:
        MutexType& m_Mutex;
        AtomicBool m_Locked;
    };

    template <Concept::MutexType... MutexTypes>
    class ScopedLock
    {
    public:
        explicit ScopedLock(MutexTypes&... mutexes) noexcept : m_Mutexes(mutexes) { Lock(mutexes...); }
        explicit ScopedLock(Tags::AdoptLock, MutexTypes&... mutexes) noexcept : m_Mutexes(mutexes) {}
        ScopedLock(const ScopedLock&) = delete;
        ScopedLock(ScopedLock&&) noexcept = default;
        ~ScopedLock() noexcept
        {
            m_Mutexes.Apply([](MutexTypes&... mutexes) noexcept -> void { (..., (void) mutexes.Unlock()); });
        };

        ScopedLock& operator=(const ScopedLock&) = delete;

    private:
        Utility::Tuple<MutexTypes&...> m_Mutexes;
    };

    template <Concept::MutexType MutexType>
    class ScopedLock<MutexType>
    {
    public:
        explicit ScopedLock(MutexType& mutex) noexcept : m_Mutex(mutex) { mutex.Lock(); }
        explicit ScopedLock(Tags::AdoptLock, MutexType& mutex) noexcept : m_Mutex(mutex) {}
        ScopedLock(const ScopedLock&) = delete;
        ScopedLock(ScopedLock&&) noexcept = default;
        ~ScopedLock() noexcept
        {
            m_Mutex.Unlock();
        };

        ScopedLock& operator=(const ScopedLock&) = delete;

    private:
        MutexType& m_Mutex;
    };

    struct CallFlag
    {
        AtomicBool Called{ false };
    };

    template<typename Function, typename... ArgTypes>
    static void CallOnce(CallFlag& flag, Function&& function, ArgTypes&&... args) noexcept
    {
        if (flag.Called.load()) return;
        flag.Called.store(true);
        function(Forward<ArgTypes>(args)...);
    }
}