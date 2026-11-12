#include "MoonCore/Memory/Memory.hpp"

#include "MoonCore/Memory/MemoryTracker.hpp"
#include "MoonCore/Utility/Algorithm.hpp"

namespace MC::Memory
{
    DataHeader* GetHeader(void* ptr) noexcept
    {
        if (!ptr) return nullptr;
        return static_cast<DataHeader*>(ptr) - 1;
    }

    void SetMemoryState(void* ptr, MemoryState state) noexcept
    {
        if (!ptr) return;
        GetHeader(ptr)->State = state;
    }

    void* Allocate(usize size, usize alignment) noexcept
    {
        constexpr usize headerSize = sizeof(DataHeader);
        const usize newAlignment = Max(alignof(DataHeader), alignment);
        const usize offset = (headerSize + newAlignment - 1) & ~(newAlignment - 1);

        void* rawAddress = _mm_malloc(size + offset, newAlignment);
        if (!rawAddress)
        {
            MC_VALIDATE(false, "Failed to allocate memory!");
            return nullptr;
        }

        void* payloadAddress = static_cast<char7*>(rawAddress) + offset;
        DataHeader* header = GetHeader(payloadAddress);
        header->Base = rawAddress;
        header->Alignment = alignment;
        header->Size = size;
        header->State = MemoryState::Default;
        header->Category = 0;
        header->Tracked = false;
        header->Trackable = MemoryTracker::IsInitialized();

        if (header->Trackable)
            MemoryTracker::OnMemoryAllocated(size);

        return payloadAddress;
    }

    void Free(void* ptr) noexcept
    {
        if (!ptr) return;
        DataHeader* header = GetHeader(ptr);
        if (header->Tracked)
            MemoryTracker::OnTrackedMemoryFreed(ptr);
        else if (header->Trackable)
            MemoryTracker::OnMemoryFreed(header->Size);

        _mm_free(header->Base);
    }

    void* Reallocate(void* ptr, usize newSize, usize alignment) noexcept
    {
        if (!ptr) 
            return Allocate(newSize, alignment == 0 ? DefaultAlignment : alignment);
        if (newSize == 0)
        {
            Free(ptr); 
            return nullptr;
        }
        if (alignment == 0) alignment = GetAlignment(ptr);

        void* newPtr = Allocate(newSize, alignment);
        if (!newPtr) return nullptr;

        memcpy_s(newPtr, newSize, ptr, Min(GetSize(ptr), newSize));

        auto newHeader = GetHeader(newPtr);
        auto oldHeader = GetHeader(ptr);
        newHeader->Category = oldHeader->Category;
        newHeader->State = oldHeader->State;

        Free(ptr);
        return newPtr;
    }

    usize GetAlignment(void* ptr) noexcept
    {
        if (!ptr) return 0;
        return (static_cast<DataHeader*>(ptr) - 1)->Alignment;
    }

    usize GetSize(void* ptr) noexcept
    {
        if (!ptr) return 0;
        return (static_cast<DataHeader*>(ptr) - 1)->Size;
    }
}