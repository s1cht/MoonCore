#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Callables/Callable.hpp"
#include "MoonCore/Containers/Array.hpp"
#include "MoonCore/Containers/Queue.hpp"
#include "Thread.hpp"
#include "ConditionVariable.hpp"
#include "Mutex.hpp"
#include "Barrier.hpp"
#include "Future.hpp"
namespace MC::Async
{
    enum class TaskPriority : u8
    {
        Low = 0,
        Medium,
        High, Critical
    };

    struct Task
    {
        Callables::Callable<void()> Function;
        TaskPriority Priority;

        bool operator<(const Task& other) const noexcept
        {
            return Priority < other.Priority;
        }
    };

    enum class IterationState : u8
    {
        Continued = 0,
        Broken,
        Returned
    };

    struct IterationValue
    {
        IterationState State = IterationState::Returned;
        IterationValue(IterationState state) : State(state) {}
    };

    namespace Parallel
    {
        static thread_local Atomic<IterationState>* g_State;

        static void Continue() noexcept { g_State->store(IterationState::Continued, std::memory_order_relaxed); }
        static void Break() noexcept { g_State->store(IterationState::Broken, std::memory_order_relaxed); }
        static void Return() noexcept { g_State->store(IterationState::Returned, std::memory_order_relaxed); }
    };

#define breakParallel       return ::MC::Async::Parallel::Break()
#define continueParallel    return ::MC::Async::Parallel::Continue()
#define returnParallel      return ::MC::Async::Parallel::Return()

    class ThreadPool
    {
    public:
        ThreadPool(u64 workerCount) noexcept;
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) noexcept = default;

        ~ThreadPool() noexcept;

    public:
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool& operator=(ThreadPool&&) noexcept = default;

    public:
        void Shutdown() noexcept;
        void Wait() noexcept;

        template<typename FunctionType, typename... ArgTypes>
        void Enqueue(TaskPriority priority, FunctionType&& task, ArgTypes&&... args) noexcept;

        template<typename IndexType = usize, typename FunctionType>
        void ParallelFor(TaskPriority priority, IndexType start, IndexType end, FunctionType task) 
            noexcept(IsNothrowInvocable<FunctionType, IndexType>);

        template<typename IndexType = usize, typename FunctionType>
        void ParallelForSync(TaskPriority priority, IndexType start, IndexType end, FunctionType task) 
            noexcept(IsNothrowInvocable<FunctionType, IndexType>);

    public:
        u64 GetQueuedTaskCount() noexcept;
        u64 GetThreadCount() const noexcept { return m_Threads.Size(); }

    public:
        static u32 CurrentThreadIndex() noexcept { return t_ThreadIndex; }

    private:
        void Worker(u32 index) noexcept;

    private:
        Array<Thread> m_Threads;
        inline static thread_local u32 t_ThreadIndex = NumericLimit<u32>::Max;
    private:
        PriorityQueue<Task> m_Tasks;
        Mutex m_QueueMutex;
        ConditionVariable m_QueueCondition;
        AtomicUsize m_ActiveTasks;

    private:
        AtomicBool m_Works;
    };

    template<typename FunctionType, typename... ArgTypes>
    void ThreadPool::Enqueue(TaskPriority priority, FunctionType&& task, ArgTypes&&... args) noexcept
    {
        {
            UniqueLock lock(m_QueueMutex);
            if (!m_Works) return;
            ++m_ActiveTasks;
            m_Tasks.Emplace(Task([task = Forward<FunctionType>(task), ...args = Forward<ArgTypes>(args)]() mutable noexcept { Invoke(Forward<FunctionType>(task), Forward<ArgTypes>(args)...); }, priority));
        }
        m_QueueCondition.NotifyOne();
    }

    template<typename IndexType, typename FunctionType>
    void ThreadPool::ParallelFor(TaskPriority priority, IndexType start, IndexType end, FunctionType task) 
        noexcept(IsNothrowInvocable<FunctionType, IndexType>)
    {
        IndexType totalItems = end - start;
        if (totalItems == 0) return;

        usize active_threads = Min(static_cast<usize>(totalItems), m_Threads.Size());

        IndexType chunkSize = static_cast<IndexType>(totalItems / active_threads);
        IndexType remainder = static_cast<IndexType>(totalItems % active_threads);

        IndexType currentStart = start;
        Memory::Reference<Atomic<IterationState>> state = Memory::MakeReference<Atomic<IterationState>>(IterationState::Continued);
        for (u32 i = 0; i < active_threads; ++i)
        {
            IndexType currentEnd = currentStart + chunkSize + (i < static_cast<u32>(remainder) ? 1 : 0);

            Enqueue(priority, [currentStart, currentEnd, task, state]() {
                Parallel::g_State = state.Get();

                for (IndexType j = currentStart; j < currentEnd; ++j)
                {
                    if (state->load(std::memory_order_relaxed) == IterationState::Broken ||
                        state->load(std::memory_order_relaxed) == IterationState::Returned) break;
                    task(j);
                }

                Parallel::g_State = nullptr;
            });

            currentStart = currentEnd;
        }
    }

    template <typename IndexType, typename FunctionType>
    void ThreadPool::ParallelForSync(TaskPriority priority, IndexType start, IndexType end, FunctionType task)
        noexcept(IsNothrowInvocable<FunctionType, IndexType>)
    {
        IndexType total_items = end - start;
        if (total_items == 0) return;

        u32 activeThreads = static_cast<u32>(Min(static_cast<usize>(total_items), m_Threads.Size()));

        IndexType chunkSize = static_cast<IndexType>(total_items / activeThreads);
        IndexType remainder = static_cast<IndexType>(total_items % activeThreads);

        Barrier barrier(activeThreads + 1);

        IndexType currentStart = start;

        Atomic state(IterationState::Continued);
        for (u32 i = 0; i < activeThreads; ++i)
        {
            IndexType currentEnd = currentStart + chunkSize + (i < static_cast<u32>(remainder) ? 1 : 0);

            Enqueue(priority, [currentStart, currentEnd, task, &state, &barrier]() {
                Parallel::g_State = &state;

                for (IndexType j = currentStart; j < currentEnd; ++j)
                {
                    if (state.load(std::memory_order_relaxed) == IterationState::Broken ||
                        state.load(std::memory_order_relaxed) == IterationState::Returned) break;
                    task(j);
                }

                Parallel::g_State = nullptr;
                barrier.ArriveAndDrop();
            });

            currentStart = currentEnd;
        }

        barrier.ArriveAndWait();
    }

    extern ThreadPool& GetThreadPool() noexcept;
}