#include "MoonCore/Memory/MemoryTracker.hpp"

#include "MoonCore/Logging/Logger.hpp"

namespace MC::Memory
{
    Scope<MemoryTracker> MemoryTracker::s_Instance = nullptr;

    constexpr StringView MemoryCategoryToStringView(MemoryCategory category) noexcept
    {
        switch (category)
        {
        case MemoryCategory::Core:
            return TEXT("Core");
        case MemoryCategory::Platform:
            return TEXT("Platform");
        case MemoryCategory::Graphics:
            return TEXT("Graphics");
        case MemoryCategory::Audio:
            return TEXT("Audio");
        case MemoryCategory::Resource:
            return TEXT("Resource");
        case MemoryCategory::Physics:
            return TEXT("Physics");
        case MemoryCategory::Application:
            return TEXT("Application");
        case MemoryCategory::UI:
            return TEXT("UI");
        default: break;
        }
        return TEXT("Unspecified");
    }

    MemoryTracker::MemoryTracker() noexcept
        : m_MemoryByCategories(static_cast<usize>(MemoryCategory::Count)), 
          m_UntrackedCount(0), m_UntrackedUsage(0)
    {
    }

    void MemoryTracker::Initialize() noexcept
    {
#ifndef MC_DIST
        if (s_Instance) return;
        s_Instance = MakeScope<MemoryTracker>();
#endif
    }

    void MemoryTracker::Shutdown() noexcept
    {
#ifndef MC_DIST
        s_Instance->ShutdownImpl();
        s_Instance.Reset();
#endif
    }

    void MemoryTracker::TrackMemory(void* ptr, MemoryCategory category, MC::StringView name,
        const Utility::Debugging::SourceData& source) noexcept
    {
#ifndef MC_DIST
        if (!s_Instance) return;
        s_Instance->TrackMemoryImpl(ptr, category, name, source);
#endif
    }

    void MemoryTracker::OnInternalAllocation(usize size, MemoryCategory category) noexcept
    {
#ifndef MC_DIST
        if (!s_Instance) return;
        s_Instance->OnInternalAllocationImpl(size, category);
#endif
    }

    void MemoryTracker::OnInternalDeletion(usize size, MemoryCategory category) noexcept
    {
#ifndef MC_DIST
        if (!s_Instance) return;
        s_Instance->OnInternalDeletionImpl(size, category);
#endif
    }

    void MemoryTracker::OnMemoryAllocated(usize size) noexcept
    {
#ifndef MC_DIST
        if (!s_Instance) return;
        s_Instance->OnMemoryAllocatedImpl(size);
#endif
    }

    void MemoryTracker::OnMemoryFreed(usize size) noexcept
    {
#ifndef MC_DIST
        if (!s_Instance) return;
        s_Instance->OnMemoryFreedImpl(size);
#endif
    }

    void MemoryTracker::OnTrackedMemoryFreed(void* ptr) noexcept
    {
#ifndef MC_DIST
        if (!s_Instance) return;
        s_Instance->OnTrackedMemoryFreedImpl(ptr);
#endif
    }

    void MemoryTracker::DumpMemoryUsage() noexcept
    {
#ifndef MC_DIST
        if (!s_Instance) return;
        s_Instance->DumpMemoryUsageImpl();
#endif
    }

    void MemoryTracker::ShutdownImpl() noexcept
    {
        DumpMemoryUsageImpl();
    }

    void MemoryTracker::TrackMemoryImpl(void* ptr, MemoryCategory category, MC::StringView name,
        const Utility::Debugging::SourceData& source) noexcept
    {
        DataHeader* header = GetHeader(ptr);
        header->Category = static_cast<u16>(category);
        header->Tracked = true;
    
        m_MemoryByCategories[static_cast<usize>(category)].Using.fetch_add(header->Size);

        Async::UniqueLock lock(m_Mutex);
        m_MemoryByCategories[static_cast<usize>(category)].Records.Insert(
            {
                reinterpret_cast<uptr>(ptr), 
                { name.ToString(), source } 
            });
    }

    void MemoryTracker::OnInternalAllocationImpl(usize size, MemoryCategory category) noexcept
    {
        m_MemoryByCategories[static_cast<usize>(category)].InternalSystemsUsing.fetch_add(size);
    }

    void MemoryTracker::OnInternalDeletionImpl(usize size, MemoryCategory category) noexcept
    {
        m_MemoryByCategories[static_cast<usize>(category)].InternalSystemsUsing.fetch_sub(size);
    }

    void MemoryTracker::OnMemoryAllocatedImpl(usize size) noexcept
    {
        ++m_UntrackedCount;
        m_UntrackedUsage.fetch_add(size);
    }

    void MemoryTracker::OnMemoryFreedImpl(usize size) noexcept
    {
        --m_UntrackedCount;
        m_UntrackedUsage.fetch_sub(size);
    }

    void MemoryTracker::OnTrackedMemoryFreedImpl(void* ptr) noexcept
    {
        Async::UniqueLock lock(m_Mutex);
        DataHeader* header = GetHeader(ptr);
        m_MemoryByCategories[static_cast<usize>(header->Category)].Records.Erase(reinterpret_cast<uptr>(ptr));
        m_MemoryByCategories[static_cast<usize>(header->Category)].Using.fetch_sub(header->Size);
    }

    // TODO: Dump into file, not into Console
    void MemoryTracker::DumpMemoryUsageImpl() noexcept
    {
        Async::SharedLock lock(m_Mutex);

        u64 untracked = m_UntrackedUsage.load(std::memory_order_relaxed);
        f32 untrackedKB = static_cast<f32>(untracked) / 1024.f;
        f32 untrackedMB = untrackedKB / 1024.f;
        f32 untrackedGB = untrackedMB / 1024.f;

        Logging::Logger::Info(Logging::Logger::Module::Memory, "Memory Dump");
        Logging::Logger::Info(Logging::Logger::Module::Memory, "Untracked memory, size: {} B, {} KB, {} MB, {} GB; count: {}", 
            untracked, untrackedKB, untrackedMB, untrackedGB, m_UntrackedCount.load(std::memory_order_relaxed));
    
        for (usize categoryType = 0; categoryType < m_MemoryByCategories.Size(); ++categoryType)
        {
            MemoryUsage& usage = m_MemoryByCategories[categoryType];
            if (usage.InternalSystemsUsing == 0 || usage.Using == 0) continue;

            Logging::Logger::Info(Logging::Logger::Module::Memory, "Category '{}': tracked: {}, internal systems: {}",
                usage.Using.load(std::memory_order_relaxed), usage.InternalSystemsUsing.load(std::memory_order_relaxed));
            for (auto& entry : usage.Records)
                Logging::Logger::Info(Logging::Logger::Module::Memory, "\t Ptr '{}': 0x{x}", entry.Value.Name, entry.Key);
        }
    }
}