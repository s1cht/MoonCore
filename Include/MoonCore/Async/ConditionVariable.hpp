#pragma once
#include "Mutex.hpp"

namespace MC::Async
{
    class ConditionVariableBase
    {
    public:
        virtual ~ConditionVariableBase() noexcept = default;

        virtual void NotifyOne() noexcept = 0;
        virtual void NotifyAll() noexcept = 0;

        virtual void Wait(Mutex* mutex) noexcept = 0;
        virtual bool Wait(Mutex* mutex, Clock::Timestep duration) noexcept = 0;

    public:
        static Memory::Scope<ConditionVariableBase> Create();
    };
    
    class ConditionVariable
    {
    public:
        ConditionVariable() noexcept : m_Mutex(Memory::MakeReference<Mutex>()), m_Variable(ConditionVariableBase::Create()) {}
        ConditionVariable(const ConditionVariable&) = delete;

    public:
        ConditionVariable& operator=(const ConditionVariable&) = delete;

    public:
        void NotifyOne() const noexcept
        {
            LockGuard lock(*m_Mutex);
            m_Variable->NotifyOne();
        }
        void NotifyAll() const noexcept
        {
            LockGuard lock(*m_Mutex);
            m_Variable->NotifyAll();
        }

        template<Concept::MutexType MutexType>
        void Wait(UniqueLock<MutexType>& lock) noexcept
        {
            const Memory::Reference<Mutex> mutex = m_Mutex;
            UniqueLock myLock(*mutex);
            lock.Unlock();
            m_Variable->Wait(mutex.Get());
            myLock.Unlock();
            lock.Lock();
        }

        template<Concept::MutexType MutexType, typename Predicate>
            requires Concept::FunctionReturns<Predicate&&, bool>
        void Wait(UniqueLock<MutexType>& lock, Predicate&& predicate) noexcept
        {
            while (!predicate())
                Wait(lock);
        }

        template<Concept::MutexType MutexType>
        bool WaitFor(UniqueLock<MutexType>& lock, Clock::Timestep duration) noexcept
        {
            const Memory::Reference<Mutex> mutex = m_Mutex;
            UniqueLock myLock(*mutex);
            lock.Unlock();
            bool unblocked = m_Variable->Wait(mutex.Get(), duration);
            myLock.Unlock();
            if (unblocked)
                lock.Lock();
            return unblocked;
        }

        template<Concept::MutexType MutexType, typename Predicate>
            requires Concept::FunctionReturns<Predicate&&, bool>
        bool WaitFor(UniqueLock<MutexType>& lock, Clock::Timestep duration,  Predicate&& predicate) noexcept
        {
            while (!predicate())
                if (!WaitFor(lock, duration))
                    return predicate();
            return true;
        }

        template<Concept::MutexType MutexType>
        bool WaitUntil(UniqueLock<MutexType>& lock, Clock::Timepoint timepoint) noexcept
        {
            return WaitFor(lock, timepoint - Clock::Time::Now());
        }

        template<Concept::MutexType MutexType, typename Predicate>
            requires Concept::FunctionReturns<Predicate&&, bool>
        bool WaitUntil(UniqueLock<MutexType>& lock, Clock::Timepoint timepoint, Predicate&& predicate) noexcept
        {
            return WaitFor(lock, timepoint - Clock::Time::Now(), Forward<Predicate&&>(predicate));
        }

    private:
        Memory::Reference<Mutex> m_Mutex;
        Memory::Scope<ConditionVariableBase> m_Variable;
    };
}