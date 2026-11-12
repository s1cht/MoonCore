#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Containers/Array.hpp"
#include "MoonCore/Containers/Queue.hpp"
#include "MoonCore/Containers/UnorderedMap.hpp"
#include "MoonCore/Memory/Allocators/AllocatorShared.hpp"

namespace MC::Memory
{
    class BuddyAllocator
    {
        struct BuddyAllocation
        {
            usize Offset;
            u8 Level;
        };

    public:
        BuddyAllocator(AllocatorSpecification specification) noexcept;
        BuddyAllocator(const BuddyAllocator&) = delete;
        BuddyAllocator(BuddyAllocator&& other) noexcept;
        ~BuddyAllocator() noexcept;

    public:
        BuddyAllocator& operator=(const BuddyAllocator&) = delete;
        BuddyAllocator& operator=(BuddyAllocator&& other) noexcept;

    public:
        AllocationError Init() noexcept;
        void DumpFreeLists() const noexcept;

        usize GetSize() const noexcept { return m_Specification.TotalSize; }
        usize GetMinBlockSize() const noexcept { return m_Specification.MinBlockSize; }

    public:
        AllocationError Allocate(usize size, OffsetAllocation& allocation) noexcept;
        AllocationError Free(const OffsetAllocation& allocation) noexcept;
        void Clear() noexcept;
        bool Resize(usize size) noexcept;

    private:
        u8 GetLevel(usize size) const noexcept;
        inline usize GetBlockSize(u8 level) const noexcept;
        usize GetBuddyOffset(usize offset, u8 level) const noexcept;

        static constexpr bool ValidateSpecification(const AllocatorSpecification& specification) noexcept;
        static constexpr u8 CalculateMaxLevel(usize totalSize, usize minBlockSize) noexcept;

    private:
        MC::Array<MC::Array<usize>> m_FreeLists;
        MC::UnorderedMap<usize, BuddyAllocation> m_Allocated;

    private:
        AllocatorSpecification m_Specification;
        u8 m_MaxLevel;
        u64 m_CurrentAllocationId;
    };

    constexpr bool BuddyAllocator::ValidateSpecification(const AllocatorSpecification& specification) noexcept
    {
        if (specification.MinBlockSize == 0) return false;
        if (specification.TotalSize <= specification.MinBlockSize) return false;
        if (!IS_POWER_OF_2(specification.MinBlockSize)) return false;
        if (!IS_POWER_OF_2(specification.TotalSize)) return false;
        return true;
    }

    constexpr u8 BuddyAllocator::CalculateMaxLevel(usize totalSize, usize minBlockSize) noexcept
    {
        u8 level = 0;
        usize block = minBlockSize;
        while (block < totalSize)
        {
            block <<= 1;
            ++level;
        }
        return level;
    }
}