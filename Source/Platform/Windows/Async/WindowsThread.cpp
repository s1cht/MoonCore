#include "WindowsThread.hpp"

#include "MoonCore/Containers/String/String.hpp"
#include "MoonCore/Logging/Logger.hpp"
#include "MoonCore/Utility/Format.hpp"

using namespace MC::Logging;

namespace MC::Windows::Async
{
    WindowsThread::WindowsThread(const MC::Async::ThreadSpecification& specification, Callables::Callable<void()>&& function) noexcept
        : m_Thread(nullptr)
    {
        m_ThreadData = nullptr;
        Init(specification, Forward<Callables::Callable<void()>>(function));
    }

    WindowsThread::~WindowsThread() noexcept
    {
        if (m_Thread) Join();
    }

    bool WindowsThread::Joinable() const noexcept
    {
        return m_Thread != nullptr;
    }

    void WindowsThread::Join() noexcept
    {
        if (m_Thread)
        {
            WaitForSingleObject(m_Thread, INFINITE);
            CloseHandle(m_Thread);
            m_Thread = nullptr;
            m_ThreadData = nullptr;
        }
    }

    void WindowsThread::Detach() noexcept
    {
        if (m_Thread) CloseHandle(m_Thread);
        m_Thread = nullptr;
        m_ThreadData = nullptr;
    }

    void WindowsThread::Resume() const noexcept
    {
        if (m_Thread) ResumeThread(m_Thread);
    }

    void WindowsThread::Suspend() const noexcept
    {
        if (m_Thread) SuspendThread(m_Thread);
    }

    void WindowsThread::Init(const MC::Async::ThreadSpecification& specification,
        Callables::Callable<void()>&& function) noexcept
    {
        ThreadProcData* data = new(std::nothrow) ThreadProcData;
        data->ThreadId = INVALID_THREAD;
        data->ThreadDataPtr = &m_ThreadData;
        data->ThreadFunction = Move(function);

        DWORD threadId;
        m_Thread = CreateThread(nullptr, specification.StackSize, ThreadProc, data, CREATE_SUSPENDED, &threadId);
        MC_RUNTIME_VALIDATE(m_Thread, "Thread creation failed!");
        data->ThreadId = static_cast<u64>(threadId);

        WideString threadName = StringToWideString(specification.ThreadName.String());
        HRESULT result = SetThreadDescription(m_Thread, threadName.String());
        if (FAILED(result))
            Logger::Warn(Logger::Module::Core,
                TEXT("Failed to set thread's [{}] name! Thread remains with default system name. HRESULT: {}"), specification.ThreadName, static_cast<i32>(result));
        if (!SetThreadPriority(m_Thread, MCThreadPriorityToWin32(specification.Priority)))
            Logger::Warn(Logger::Module::Core,
                TEXT("Failed to set thread's [{}] priority! Thread remains with the default system priority. Win32 error: {}"), specification.ThreadName, static_cast<i32>(GetLastError()));
        if (SetThreadAffinityMask(m_Thread, static_cast<DWORD_PTR>(specification.CoreMask)) == 0)
            Logger::Warn(Logger::Module::Core,
                TEXT("Failed to set thread's [{}] core mask! Thread core mask remains as default. Win32 error: {}"), specification.ThreadName, static_cast<i32>(GetLastError()));
        if (!SetThreadPriorityBoost(m_Thread, specification.DisablePriorityBoost))
            Logger::Warn(Logger::Module::Core,
                TEXT("Failed to set thread's [{}] priority boost! Win32 error: {}"), specification.ThreadName, static_cast<i32>(GetLastError()));

        if (!specification.Suspended)
            ResumeThread(m_Thread);
    }

    DWORD WindowsThread::ThreadProc(LPVOID lpParameter) noexcept
    {
        ThreadProcData* data = static_cast<ThreadProcData*>(lpParameter);
        MC::Async::CurrentThread::Data().ThreadId = data->ThreadId;
        (*data->ThreadDataPtr) = &MC::Async::CurrentThread::Data();

        data->ThreadFunction();

        delete data;
        return 0;
    }
}

namespace MC::Async
{
    void MC::Async::CurrentThread::Yield() noexcept
    {
        YieldProcessor();
    }

    void MC::Async::CurrentThread::Sleep(Clock::Timestep duration) noexcept
    {
        ::Sleep(static_cast<DWORD>(duration.AsMilliseconds()));
    }

    void MC::Async::CurrentThread::SleepUntil(Clock::Timepoint timepoint) noexcept
    {
        const Clock::Timepoint now = Clock::Time::Now();
        if (timepoint <= now) return Yield();
        return Sleep(timepoint - now);
    }

    u64 CurrentThread::ID() noexcept
    {
        return static_cast<u64>(GetCurrentThreadId());
    }

    Memory::Reference<MC::Async::ThreadBase, Memory::ReferenceMode::ThreadSafe> MC::Async::ThreadBase::Create(
        const MC::Async::ThreadSpecification& specification, Callables::Callable<void()>&& function) noexcept
    {
        return Memory::MakeReference<Windows::Async::WindowsThread>(specification, Forward<Callables::Callable<void()>>(function));
    }
}