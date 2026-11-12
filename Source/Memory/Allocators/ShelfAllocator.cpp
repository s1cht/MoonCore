#include "MoonCore/Memory/Allocators/Offset/ShelfAllocator.hpp"

namespace MC::Memory
{
    ShelfAllocator::ShelfAllocator(u32 maxWidthHeight) noexcept
        : m_NextShelfBegin(0), m_MaxDimensions(maxWidthHeight), m_Generation(0)
    {}

    OffsetAllocation2D ShelfAllocator::Allocate(Math::Resolution2D<u32> size) noexcept
    {
        if (size.x > m_MaxDimensions || size.y >= m_MaxDimensions || size.x == 0 || size.y == 0)
            return OffsetAllocation2D::Null();

        usize bestIndex = ~0ull;
        u32 smallestWaste = NumericLimit<i32>::Max;

        for (usize i = 0; i < m_Shelves.Size(); ++i)
        {
            if (size.y > m_Shelves[i].Range.Length) continue;
            if (FindSuitableRange(i, size.x) == 0xFFFFFFFFFFFFFFFF) continue;

            u32 waste = m_Shelves[i].Range.Length - size.y;
            if (waste < smallestWaste)
            {
                bestIndex = i;
                smallestWaste = waste;
            }
        }

        if (bestIndex == ~0ull)
        {
            if (m_NextShelfBegin + size.y > m_MaxDimensions)
                return OffsetAllocation2D::Null();

            m_Shelves.PushBack(Shelf{{ m_NextShelfBegin, size.y }, { Region{0, m_MaxDimensions} }});
            m_NextShelfBegin += size.y;
            bestIndex = m_Shelves.Size() - 1;
        }

        Shelf& shelf = m_Shelves[bestIndex];
        usize s = FindSuitableRange(bestIndex, size.x);
        Region& region = shelf.FreeRanges[s];

        Math::Rect2D rect{ {region.Offset, shelf.Range.Offset}, size };
        region.Offset += size.x;
        region.Length -= size.x;
        if (size.x == 0)
            shelf.FreeRanges.Erase(shelf.FreeRanges.Begin() + s);

        return { AllocationBase(bestIndex, m_Generation, this), rect };
    }

    void ShelfAllocator::Free(const OffsetAllocation2D& allocation) noexcept
    {
        if (allocation == nullptr) return;
        if (m_Generation < allocation.Generation) return;

        Shelf& shelf = m_Shelves[allocation.AllocationID];
        Array<Region>& region = shelf.FreeRanges;

        size_t i = 0;
        while (i < region.Size() && region[i].Offset < allocation.Rect.Offset.x) ++i;
        region.Insert(region.CBegin() + i, Region{ allocation.Rect.Offset.x, allocation.Rect.Extent.x });

        if (i + 1 < region.Size() && region[i].Offset + region[i].Length == region[i + 1].Offset)
        {
            region[i].Length += region[i + 1].Length;
            region.Erase(region.Begin() + i + 1);
        }
        if (i > 0 && region[i - 1].Offset + region[i - 1].Length == region[i].Offset)
        {
            region[i - 1].Length += region[i].Length;
            region.Erase(region.Begin() + i);
        }

        while (!m_Shelves.Empty() && IsEmpty(m_Shelves.Back()))
        {
            m_NextShelfBegin = m_Shelves.Back().Range.Offset;
            m_Shelves.PopBack();
        }
    }

    void ShelfAllocator::Clear() noexcept
    {
        m_Shelves.Clear();
        ++m_Generation;
    }

    bool ShelfAllocator::IsEmpty(const Shelf& shelf) const noexcept
    {
        return shelf.FreeRanges.Size() == 1 && shelf.FreeRanges[0].Offset == 0 && shelf.FreeRanges[0].Length == m_MaxDimensions;
    }

    usize ShelfAllocator::FindSuitableRange(usize shelf, usize width) const noexcept
    {
        for (usize i = 0; i < m_Shelves[shelf].FreeRanges.Size(); ++i)
            if (m_Shelves[shelf].FreeRanges[i].Length >= width) return i;
        return 0xFFFFFFFFFFFFFFFF;
    }
}
