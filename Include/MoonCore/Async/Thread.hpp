#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Callables/Callable.hpp"
#include "MoonCore/Clock/Time.hpp"
#include "MoonCore/Memory/Pointers/Reference.hpp"
#include "MoonCore/Containers/String/StringView.hpp"
#include "Atomic.hpp"

namespace MC::Async
{
    enum class ThreadPriority : u8
    {
        Normal = 0,
        AboveNormal,
        BelowNormal,
        Highest,
        Lowest,
    };

    class CurrentThread
    {
    public:
        struct ThreadData
        {
            u64 ThreadId;
            AtomicBool StopRequested{ false };
        };

    public:
        static void Yield() noexcept;
        static void Sleep(Clock::Timestep duration) noexcept;
        static void SleepUntil(Clock::Timepoint timepoint) noexcept;
        static u64 ID() noexcept;
        static bool StopRequested() noexcept { return t_Data.StopRequested.load(std::memory_order_relaxed); }
        static ThreadData& Data() noexcept { return t_Data; }

    private:
        static thread_local ThreadData t_Data;
    
    private:
        friend class ThreadBase;
    };

    struct ThreadSpecification
    {
        StringView ThreadName;
        usize StackSize = 0; // Default
        u64 CoreMask = 0xFFFFFFFFFFFFFFFF;
        ThreadPriority Priority = ThreadPriority::Normal;
        bool Suspended = false;
        bool DisablePriorityBoost = true;
    };

    class ThreadBase
    {
    public:
        static constexpr u64 INVALID_THREAD = ~0ull;

    public:
        virtual ~ThreadBase() noexcept = default;

        virtual bool Joinable() const noexcept = 0;

        virtual void Join() noexcept = 0;
        virtual void Detach() noexcept = 0;
        virtual void Resume() const noexcept = 0;
        virtual void Suspend() const noexcept = 0;
    
    public:
        u64 ThreadId() const noexcept;
        bool RequestStop() const noexcept;

    public:
        static Memory::Reference<ThreadBase, Memory::ReferenceMode::ThreadSafe> Create(const ThreadSpecification& specification, Callables::Callable<void()>&& function) noexcept;

    protected:
        mutable CurrentThread::ThreadData* m_ThreadData = nullptr;
    };

    class Thread final
    {
    public:
        Thread() noexcept = default;
        Thread(const Thread&) = delete;

        Thread(Thread&&) noexcept = default;
        
        template <typename F, typename... ArgTypes>
            requires IsNothrowInvocable<F&&>
        explicit Thread(const ThreadSpecification& specification, F&& function) noexcept
        {
            m_Thread = ThreadBase::Create(specification, Callables::Callable<void()>(Forward<F&&>(function)));
        }
        template <typename F, typename... ArgTypes>
        explicit Thread(const ThreadSpecification& specification, F&& function, ArgTypes&&... args) noexcept
        {
            m_Thread = ThreadBase::Create(specification, Callables::Bind(Forward<F>(function), Forward<ArgTypes>(args)...));
        }
        explicit Thread(const ThreadSpecification& specification, Callables::Callable<void()>&& function) noexcept
        {
            m_Thread = ThreadBase::Create(specification, Forward<Callables::Callable<void()>>(function));
        }

        ~Thread() noexcept;

    public:
        Thread& operator=(const Thread&) = delete;
        Thread& operator=(Thread&&) noexcept = default;

    public:
        MC_NODISCARD bool Joinable() const noexcept
        {
            if (m_Thread) return m_Thread->Joinable();
            return false;
        }
        MC_NODISCARD u64 ThreadId() const noexcept
        {
            if (m_Thread) return m_Thread->ThreadId();
            return ThreadBase::INVALID_THREAD;
        }

        void Join() const noexcept
        {
            if (m_Thread) m_Thread->Join();
        }
        void Detach() noexcept
        {
            if (m_Thread) m_Thread->Detach();
        }
        MC_NODISCARD bool RequestStop() const noexcept
        {
            if (m_Thread) return m_Thread->RequestStop();
            return false;
        }

    public:
        void Swap(Thread& other) noexcept
        {
            auto temp = m_Thread;
            m_Thread = other.m_Thread;
            other.m_Thread = temp;
        }

    private:
        Memory::Reference<ThreadBase, Memory::ReferenceMode::ThreadSafe> m_Thread;
    };
}