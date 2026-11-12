#pragma once
#include "MoonCore/Types.hpp"
#include "MoonCore/Containers/String/String.hpp"
#include "MoonCore/Math/Rect.hpp"

namespace MC::Memory
{
    enum class AllocationError : u8
    {
        Success,
        // Allocator errors
        IncorrectSpecification,
        // Allocation errors
        OutOfMemory, TooBigRequest, TooSmallRequest,
        // Free errors
        OutOfRange, InvalidArgument
    };

    struct AllocatorSpecification
    {
        usize MinBlockSize = 2;
        usize TotalSize = 1024;
        MC::String DebugName = "Allocator";
    };

    struct AllocationBase
    {
        constexpr AllocationBase() noexcept = default;
        AllocationBase(usize allocationID, usize generation, void* allocator) noexcept 
            : AllocationID(allocationID), Generation(generation), Allocator(MakeAnAllocatorSignature(allocator)) {}
        AllocationBase(const AllocationBase&) = delete;
        AllocationBase(AllocationBase&&) noexcept = default;
 
        AllocationBase& operator=(const AllocationBase&) = delete;
        AllocationBase& operator=(AllocationBase&&) noexcept = default;

        usize AllocationID = 0xFFFFFFFFFFFFFFFF;
        usize Generation   = 0xFFFFFFFFFFFFFFFF;
        uptr Allocator     = 0xFFFFFFFFFFFFFFFF;
    
        static uptr MakeAnAllocatorSignature(void* allocatorAddress) noexcept { return reinterpret_cast<uptr>(allocatorAddress); }
    };
    
    struct IndexAllocation : AllocationBase
    {
        constexpr IndexAllocation() noexcept = default;
        IndexAllocation(AllocationBase&& base, usize index, usize indexCount) noexcept 
            : AllocationBase(Move(base)), Index(index), IndexCount(indexCount) {}
        usize Index = 0xFFFFFFFFFFFFFFFF;
        usize IndexCount = 0x0000000000000000;

        operator bool() const noexcept
        {
            return Index != 0xFFFFFFFFFFFFFFFF && IndexCount != 0;
        }

        bool operator==(nullptr_t) const noexcept
        {
            return Index != 0xFFFFFFFFFFFFFFFF && IndexCount != 0;
        }
    };

    struct OffsetAllocation : AllocationBase
    {
        constexpr OffsetAllocation() noexcept = default;
        OffsetAllocation(AllocationBase&& base, usize offset) noexcept 
            : AllocationBase(Move(base)), Offset(offset) {}
        usize Offset = 0xFFFFFFFFFFFFFFFF;

        operator bool() const noexcept
        {
            return Offset != 0xFFFFFFFFFFFFFFFF;
        }

        bool operator==(nullptr_t) const noexcept
        {
            return Offset != 0xFFFFFFFFFFFFFFFF;
        }
    };

    struct OffsetAllocation2D : public AllocationBase
    {
        constexpr OffsetAllocation2D() noexcept = default;
        OffsetAllocation2D(AllocationBase&& base, Math::Rect2D rect) noexcept
            : AllocationBase(Move(base)), Rect(rect) {}
        Math::Rect2D Rect;

        operator bool() const noexcept
        {
            return Rect.Extent.x != 0xFFFFFFFF && Rect.Extent.y != 0xFFFFFFFF && Rect.Offset.x != 0xFFFFFFFF && Rect.Offset.y != 0xFFFFFFFF;
        }

        bool operator==(nullptr_t) const noexcept
        {
            return Rect.Extent.x == 0xFFFFFFFF || Rect.Extent.y == 0xFFFFFFFF || Rect.Offset.x == 0xFFFFFFFF || Rect.Offset.y == 0xFFFFFFFF;
        }

        static OffsetAllocation2D Null() noexcept 
        {
            return OffsetAllocation2D(AllocationBase(0xFFFFFFFFFFFFFFFF, 0, nullptr),
                Math::Rect2D{ {0xFFFFFFFF, 0xFFFFFFFF}, {0xFFFFFFFF, 0xFFFFFFFF} });
        }
    };
}
