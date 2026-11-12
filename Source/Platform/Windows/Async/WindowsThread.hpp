#pragma once
#include "MoonCore/Async/Thread.hpp"
#include "Platform/Windows/WindowsInclude.hpp"

namespace MC::Windows::Async
{
    struct ThreadProcData
    {
        Callables::Callable<void()> ThreadFunction;
        MC::Async::CurrentThread::ThreadData** ThreadDataPtr;
        u64 ThreadId;
    };

    class WindowsThread final : public MC::Async::ThreadBase
    {
    public:
        WindowsThread(const MC::Async::ThreadSpecification& specification, Callables::Callable<void()>&& function) noexcept;
        ~WindowsThread() noexcept override;

    public:
        bool Joinable() const noexcept override;
        void Join() noexcept override;
        void Detach() noexcept override;
        void Resume() const noexcept override;
        void Suspend() const noexcept override;

    private:
        void Init(const MC::Async::ThreadSpecification& specification, Callables::Callable<void()>&& function) noexcept;
    
    private:
        static constexpr int MCThreadPriorityToWin32(MC::Async::ThreadPriority priority) noexcept;
        static DWORD WINAPI ThreadProc(LPVOID lpParameter) noexcept;

    private:
        HANDLE m_Thread;
    };

    constexpr int WindowsThread::MCThreadPriorityToWin32(MC::Async::ThreadPriority priority) noexcept
    {
        switch (priority) 
        {
            case MC::Async::ThreadPriority::Normal:           return THREAD_PRIORITY_NORMAL;
            case MC::Async::ThreadPriority::AboveNormal:      return THREAD_PRIORITY_ABOVE_NORMAL;
            case MC::Async::ThreadPriority::BelowNormal:      return THREAD_PRIORITY_BELOW_NORMAL;
            case MC::Async::ThreadPriority::Highest:          return THREAD_PRIORITY_HIGHEST;
            case MC::Async::ThreadPriority::Lowest:           return THREAD_PRIORITY_LOWEST;
        }
        return THREAD_PRIORITY_NORMAL;
    }
}