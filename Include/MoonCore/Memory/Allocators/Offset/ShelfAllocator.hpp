#pragma once
#include "MoonCore/Types.hpp"
#include "MoonCore/Containers/Array.hpp"
#include "MoonCore/Memory/Allocators/AllocatorShared.hpp"

namespace MC::Memory
{
    class ShelfAllocator
    {
        struct Region
        {
            u32 Offset, Length;
        };

        struct Shelf
        {
            Region Range;
            Array<Region> FreeRanges;
        };

    public:
        ShelfAllocator(u32 maxWidthHeight) noexcept;
        ShelfAllocator(const ShelfAllocator&) = delete;
        ShelfAllocator(ShelfAllocator&& other) noexcept = default;
        ~ShelfAllocator() noexcept = default;

    public:
        ShelfAllocator& operator=(const ShelfAllocator&) = delete;
        ShelfAllocator& operator=(ShelfAllocator&& other) noexcept = default;

    public:
        OffsetAllocation2D Allocate(Math::Resolution2D<u32> size) noexcept;
        void Free(const OffsetAllocation2D& allocation) noexcept;
        void Clear() noexcept;

    private:
        bool IsEmpty(const Shelf& shelf) const noexcept;

        usize FindSuitableRange(usize shelf, usize width) const noexcept;

    private:
        usize m_Generation;
        Array<Shelf> m_Shelves;
        u32 m_NextShelfBegin;
        u32 m_MaxDimensions;
    };
}
