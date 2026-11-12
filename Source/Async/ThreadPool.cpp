#include "MoonCore/Async/ThreadPool.hpp"
#include "MoonCore/Containers/String/String.hpp"
#include "MoonCore/Utility/Format.hpp"

namespace MC::Async
{
    ThreadPool::ThreadPool(u64 workerCount) noexcept
        : m_ActiveTasks(0), m_Works(true)
    {
        constexpr char8 WorkerName[] = TEXT("Worker %d");

        m_Threads.Reserve(workerCount);

        ThreadSpecification specification;
        for (u32 i = 0; i < workerCount; ++i)
        {
            MC::String name = Formatter::Format(WorkerName, i);
            specification.ThreadName = name.ToStringView();
            m_Threads.EmplaceBack(specification, [this, i]() noexcept { Worker(i); });
        }
    }

    ThreadPool::~ThreadPool() noexcept
    {
        Shutdown();
    }

    void ThreadPool::Shutdown() noexcept
    {
        {
            UniqueLock lock(m_QueueMutex);
            m_Works = false;
        }
        m_QueueCondition.NotifyAll();
        for (auto& thread : m_Threads)
            if (thread.Joinable()) thread.Join();
    }

    void ThreadPool::Wait() noexcept
    {
        UniqueLock lock(m_QueueMutex);
        m_QueueCondition.Wait(lock, [this]() noexcept {
            return m_Tasks.Empty() && m_ActiveTasks == 0;
            });
    }

    u64 ThreadPool::GetQueuedTaskCount() noexcept
    {
        LockGuard lock(m_QueueMutex);
        return m_Tasks.Size();
    }

    void ThreadPool::Worker(u32 index) noexcept
    {
        t_ThreadIndex = index;
        while (m_Works)
        {
            Callables::Callable<void()> task;
            {
                UniqueLock lock(m_QueueMutex);

                m_QueueCondition.Wait(lock, [this]() noexcept {
                    return !m_Tasks.Empty() || !m_Works;
                    });

                if (!m_Works && m_Tasks.Empty())
                    return;

                task = m_Tasks.Top().Function;
                m_Tasks.Pop();
            }
            task();

            if (--m_ActiveTasks == 0)
                m_QueueCondition.NotifyAll();
        }
    }
}
