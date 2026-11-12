#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Containers/StackArray.hpp"
#include "MoonCore/Containers/String.hpp"
#include "Pointers/Scope.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "MoonCore/Async/Mutex.hpp"
#include "MoonCore/Containers/UnorderedMap.hpp"

namespace MC::Memory
{
    enum class MemoryCategory : u16
    {
        Uncategorized = 0,
        Core, Platform, Graphics, Audio,
        Resource, Physics, Application, UI,
        Count
    };

    class MemoryTracker
    {
        struct MemoryRecord
        {
            MC::String Name;
            Utility::Debugging::SourceData Source;
        };

        struct MemoryUsage
        {
            Async::AtomicUsize Using = 0;
            MC::UnorderedMap<uptr, MemoryRecord> Records;
            Async::AtomicUsize InternalSystemsUsing = 0;
        };

    public:
        MemoryTracker() noexcept;
        ~MemoryTracker() noexcept = default;

    public:
        static MemoryTracker& Get() noexcept
        {
            return *s_Instance;
        }

        static bool IsInitialized() noexcept
        {
#ifdef MC_DIST
            return true;
#endif
            return s_Instance;
        }

        static void Initialize() noexcept;
        static void Shutdown() noexcept;

        static void TrackMemory(void* ptr, MemoryCategory category = MemoryCategory::Uncategorized, 
            MC::StringView name = "Unknown", const Utility::Debugging::SourceData& source = Utility::Debugging::SourceData()) noexcept;
        template<typename T, Concept::Deleter<T> D>
        static void TrackMemory(Scope<T, D>& ptr, MemoryCategory category = MemoryCategory::Uncategorized,
            MC::StringView name = "Unknown", const Utility::Debugging::SourceData& source = Utility::Debugging::SourceData()) noexcept
        {
            TrackMemory(ptr.Get(), category, name, source);
        }
        template<typename T, ReferenceMode Mode, Concept::Deleter<T> D>
        static void TrackMemory(Reference<T, Mode, D>& ptr, MemoryCategory category = MemoryCategory::Uncategorized,
            MC::StringView name = "Unknown", const Utility::Debugging::SourceData& source = Utility::Debugging::SourceData()) noexcept
        {
            TrackMemory(ptr.Get(), category, name, source);
        }

        static void OnInternalAllocation(usize size, MemoryCategory category = MemoryCategory::Uncategorized) noexcept;
        static void OnInternalDeletion(usize size, MemoryCategory category = MemoryCategory::Uncategorized) noexcept;
        static void OnMemoryAllocated(usize size) noexcept;
        static void OnMemoryFreed(usize size) noexcept;
        static void OnTrackedMemoryFreed(void* ptr) noexcept;

        static void DumpMemoryUsage() noexcept;

    private:
        void ShutdownImpl() noexcept;

        void TrackMemoryImpl(void* ptr, MemoryCategory category,
            MC::StringView name, const Utility::Debugging::SourceData& source) noexcept;

        void OnInternalAllocationImpl(usize size, MemoryCategory category = MemoryCategory::Uncategorized) noexcept;
        void OnInternalDeletionImpl(usize size, MemoryCategory category = MemoryCategory::Uncategorized) noexcept;
        void OnMemoryAllocatedImpl(usize size) noexcept;
        void OnMemoryFreedImpl(usize size) noexcept;
        void OnTrackedMemoryFreedImpl(void* ptr) noexcept;

        void DumpMemoryUsageImpl() noexcept;

    private:
        static MC::Memory::Scope<MemoryTracker> s_Instance;

    private:
        MC::Async::SharedMutex m_Mutex;
        MC::StackArray<MemoryUsage, static_cast<usize>(MemoryCategory::Count)> m_MemoryByCategories;

    private:
        Async::AtomicU64 m_UntrackedCount;
        Async::AtomicUsize m_UntrackedUsage;
    };
}