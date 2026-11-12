#include "MoonCore/Memory/Allocators/Offset/BuddyAllocator.hpp"

#include "MoonCore/Logging/Logger.hpp"
#include "MoonCore/Utility/Algorithm.hpp"

using namespace MC::Logging;

namespace MC::Memory
{
    BuddyAllocator::BuddyAllocator(AllocatorSpecification specification) noexcept
        : m_Specification(Move(specification)), m_MaxLevel(0), m_CurrentAllocationId(0)
    {
        MC_VALIDATE(Init() == AllocationError::Success, "Failed to construct buddy allocator!");
    }

    BuddyAllocator::BuddyAllocator(BuddyAllocator&& other) noexcept
    {
        m_FreeLists = Move(other.m_FreeLists);
        m_Allocated = Move(other.m_Allocated);
        m_Specification = Move(other.m_Specification);
        m_MaxLevel = Move(other.m_MaxLevel);
        m_CurrentAllocationId = Move(other.m_CurrentAllocationId);
    }

    BuddyAllocator::~BuddyAllocator() noexcept
    {
        Clear();
    }

    BuddyAllocator& BuddyAllocator::operator=(BuddyAllocator&& other) noexcept
    {
        if (&other != this)
        {
            m_FreeLists = Move(other.m_FreeLists);
            m_Allocated = Move(other.m_Allocated);
            m_Specification = Move(other.m_Specification);
            m_MaxLevel = Move(other.m_MaxLevel);
            m_CurrentAllocationId = Move(other.m_CurrentAllocationId);
        }
        return *this;
    }

    AllocationError BuddyAllocator::Init() noexcept
    {
        if (!ValidateSpecification(m_Specification))
        {
            Logger::Error(Logger::Module::Memory, TEXT("Buddy allocator [{0}] can't be initialized with given specification!"), m_Specification.DebugName);
            return AllocationError::IncorrectSpecification;
        }

        m_MaxLevel = CalculateMaxLevel(m_Specification.TotalSize, m_Specification.MinBlockSize);
        m_FreeLists = Array<Array<usize>>(m_MaxLevel + 1);
        for (u8 i = 0; i <= m_MaxLevel; ++i) m_FreeLists[i].Clear();
        m_FreeLists[m_MaxLevel].EmplaceBack(0);

        m_Allocated.Clear();

        return AllocationError::Success;
    }

    void BuddyAllocator::DumpFreeLists() const noexcept
    {
        for (u8 i = 0; i <= m_MaxLevel; ++i) 
            Logger::Info(Logger::Module::Memory, "Level {0}: {1} blocks\n", i, m_FreeLists[i].Size());
    }

    AllocationError BuddyAllocator::Allocate(usize size, OffsetAllocation& allocation) noexcept
    {
        if (size == 0) return AllocationError::TooSmallRequest;
        if (size > m_Specification.TotalSize) return AllocationError::TooBigRequest;

        u8 level = GetLevel(size);

        for (u8 i = level; i <= m_MaxLevel; ++i)
        {
            if (m_FreeLists[i].Empty())
                continue;

            usize offset = m_FreeLists[i].Back();
            m_FreeLists[i].PopBack();

            u8 cur = i;
            while (cur > level)
            {
                --cur;
                usize buddyOffset = offset + GetBlockSize(cur);
                m_FreeLists[cur].EmplaceBack(buddyOffset);
            }

            m_Allocated.Insert(m_CurrentAllocationId, { offset, level });
            allocation = OffsetAllocation{};
            allocation.Offset = offset;
            allocation.AllocationID = m_CurrentAllocationId;

            ++m_CurrentAllocationId;

            return AllocationError::Success;
        }

        return AllocationError::OutOfMemory;
    }

    AllocationError BuddyAllocator::Free(const OffsetAllocation& allocation) noexcept
    {
        if (allocation.Offset >= m_Specification.TotalSize) return AllocationError::OutOfRange;

        auto it = m_Allocated.Find(allocation.AllocationID);
        if (it == m_Allocated.end())
        {
            Logger::Debug(Logger::Module::Memory, Logger::Severity::Warn, "Free failed, allocation doesn't exist in allocator!");
            return AllocationError::OutOfRange;
        }
        usize level = it->Value.Level;
        usize curOffset = it->Value.Offset;
        m_Allocated.Erase(it);

        while (level < m_MaxLevel)
        {
            usize buddyOffset = GetBuddyOffset(curOffset, static_cast<u8>(level));
            auto& list = m_FreeLists[level];
            auto buddy = Find(list.begin(), list.end(), buddyOffset);
            if (buddy == list.end())
                break;

            list.Erase(buddy);
            curOffset = Min(curOffset, buddyOffset);
            level++;
        }

        m_FreeLists[level].EmplaceBack(curOffset);

        return AllocationError::Success;
    }

    void BuddyAllocator::Clear() noexcept
    {
        for (auto& list : m_FreeLists)
            list.Clear();

        if (!m_FreeLists.Empty()) m_FreeLists[m_MaxLevel].EmplaceBack(0);
        m_Allocated.Clear();
    }

    bool BuddyAllocator::Resize(usize size) noexcept
    {
        if (size <= m_Specification.TotalSize) return true;

        if (!IS_POWER_OF_2(size))
        {
            Logger::Error(Logger::Module::Memory, "BuddyAllocator [{0}]::Resize: Size must be a power of 2", m_Specification.DebugName);
            return false;
        }

        u8 oldMaxLevel = m_MaxLevel;

        m_Specification.TotalSize = size;
        m_MaxLevel = CalculateMaxLevel(m_Specification.TotalSize, m_Specification.MinBlockSize);

        if (m_FreeLists.Size() < m_MaxLevel + 1) 
            m_FreeLists.Resize(m_MaxLevel + 1);

        for (u8 i = oldMaxLevel; i < m_MaxLevel; ++i)
            m_FreeLists[i].EmplaceBack(GetBlockSize(i));

        return true;
    }

    u8 BuddyAllocator::GetLevel(usize size) const noexcept
    {
        usize block = m_Specification.MinBlockSize;
        u8 level = 0;
        while (block < size)
        {
            block <<= 1;
            ++level;
        }
        return level;
    }

    usize BuddyAllocator::GetBlockSize(u8 level) const noexcept
    {
        return m_Specification.MinBlockSize << level;
    }

    usize BuddyAllocator::GetBuddyOffset(usize offset, u8 level) const noexcept
    {
        usize size = GetBlockSize(level);
        MC_ASSERT((offset % size) == 0, Logger::Module::Memory, TEXT("Offset misaligned for buddy computation!"));
        return offset ^ size;
    }
}