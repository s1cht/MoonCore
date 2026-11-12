#pragma once
#include "MoonCore/Containers/Map.hpp"
#include "MoonCore/Containers/Multimap.hpp"
#include "MoonCore/Memory/Allocators/AllocatorShared.hpp"

namespace MC::Memory
{
    class FreeListAllocator
    {
    public:
        using SizeType = usize;
        using OffsetType = usize;

        using SizeList = Multimap<SizeType, OffsetType>;
        using OffsetList = Map<OffsetType, SizeList::Iterator>;

    public:
        FreeListAllocator(usize maxSize) noexcept;

    public:
        bool HasSpaceFor(usize indexCount) const noexcept { return m_FreeListBySize.LowerBound(indexCount) != m_FreeListBySize.End(); }
        usize FreeIndexCount() const noexcept { return m_FreeIndexCount; }
        IndexAllocation Allocate(usize indices) noexcept;
        void Free(const IndexAllocation& allocation) noexcept;
        void Resize(usize newMaxSize) noexcept;

    private:
        void AddNewBlock(usize offset, usize indexCount) noexcept;
        void FreeBlock(usize offset, usize indexCount) noexcept;

    private:
        OffsetList m_FreeListByOffset;
        SizeList m_FreeListBySize;

        usize m_FreeIndexCount;
        usize m_MaxSize;
    };
}
