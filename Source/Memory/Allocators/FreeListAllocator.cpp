#include "MoonCore/Memory/Allocators/Index/FreeListAllocator.hpp"

#include "MoonCore/Logging/Logger.hpp"

namespace MC::Memory
{
    FreeListAllocator::FreeListAllocator(usize maxSize) noexcept
        : m_MaxSize(maxSize)
    {
        m_FreeIndexCount = maxSize;
        AddNewBlock(0, maxSize);
    }

    IndexAllocation FreeListAllocator::Allocate(usize indices) noexcept
    {
        if (indices > m_FreeIndexCount)
            return {};
        
        SizeList::Iterator smallestBlock =  m_FreeListBySize.LowerBound(indices);
        if (smallestBlock == m_FreeListBySize.End())
            return {};

        SizeType blockSize = smallestBlock->Key;
        OffsetType offset = smallestBlock->Value;

        OffsetList::Iterator offsetIterator = m_FreeListByOffset.Find(offset);
        m_FreeListBySize.Erase(smallestBlock);
        m_FreeListByOffset.Erase(offsetIterator);

        OffsetType newOffset = offset + indices;
        SizeType newSize = blockSize - indices;

        if (newSize > 0)
            AddNewBlock(newOffset, newSize);

        m_FreeIndexCount -= indices;

        return { AllocationBase(0, 0, this), offset, indices };
    }

    void FreeListAllocator::Free(const IndexAllocation& allocation) noexcept
    {
        if (allocation.Allocator != AllocationBase::MakeAnAllocatorSignature(this)) return;
        FreeBlock(allocation.Index, allocation.IndexCount);
    }

    void FreeListAllocator::Resize(usize newMaxSize) noexcept
    {
        return;
    }

    void FreeListAllocator::AddNewBlock(usize offset, usize indexCount) noexcept
    {
        Utility::Pair<SizeList::Iterator, bool> sizeIterator = m_FreeListBySize.
            Emplace(indexCount, offset);
        m_FreeListByOffset.Emplace(offset, sizeIterator.Key);
    }

    void FreeListAllocator::FreeBlock(usize offset, usize indexCount) noexcept
    {
        OffsetList::Iterator nextBlock =  m_FreeListByOffset.UpperBound(offset);
        OffsetList::Iterator prevBlock = nextBlock;

        if (prevBlock != m_FreeListByOffset.Begin())
            --prevBlock;
        else
            prevBlock = m_FreeListByOffset.End();

        m_FreeIndexCount += indexCount;
    
        if (prevBlock != m_FreeListByOffset.end() &&
            offset == prevBlock->Key + prevBlock->Value->Key)
        {
            offset = prevBlock->Key;
            indexCount += prevBlock->Value->Key;

            m_FreeListBySize.Erase(prevBlock->Value);
            m_FreeListByOffset.Erase(prevBlock);
        }

        if (nextBlock != m_FreeListByOffset.end() &&
            offset + indexCount == nextBlock->Key)
        {
            indexCount += nextBlock->Value->Key;

            m_FreeListBySize.Erase(nextBlock->Value);
            m_FreeListByOffset.Erase(nextBlock);
        }

        AddNewBlock(offset, indexCount);
    }
}