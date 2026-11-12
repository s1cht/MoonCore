#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Types.hpp"
#include "MoonCore/Utility/Debug.hpp"

namespace MC::Concept
{    
	template <class T>
	concept HeapAllocatorType = requires
	{
		typename T::ValueType;
	} && IsTrivial<T> || requires
    {
		IsNothrowDefaultConstructible<T>;
        IsNothrowDestructible<T>;
		IsNothrowCopyConstructible<T>;
		IsNothrowMoveConstructible<T>;
    }
	&& requires(T allocator, usize count, typename T::DataType* ptr)
	{
		{ allocator.Allocate() }			 noexcept -> ConvertibleTo<typename T::DataType*>;
		{ allocator.Allocate(count) }        noexcept -> ConvertibleTo<typename T::DataType*>;
		{ allocator.Deallocate(ptr) }        noexcept -> ConvertibleTo<void>;
		{ allocator.Deallocate(ptr, count) } noexcept -> ConvertibleTo<void>;
	};

	template <class T>
	concept OffsetAllocatorType = true;
}

namespace MC::Memory
{
	template <typename DataType, class... ArgTypes>
		requires IsNothrowConstructible<DataType, ArgTypes...>
    static constexpr void Construct(DataType* ptr, ArgTypes&&... args) noexcept
	{
		new (static_cast<void*>(ptr)) DataType(Forward<ArgTypes>(args)...);
	}

	template <typename DataType>
        requires IsNothrowDestructible<DataType>
	static constexpr void Destroy(DataType* ptr) noexcept
	{
		if constexpr (!IsTriviallyDestructible<DataType>)
        {
			ptr->~DataType();
        }
	}

	inline static constexpr usize DefaultAlignment = alignof(MaxAlignT);

	enum class MemoryState : u8
	{
	    Default = 0,
		ContainerMemory,
		PoolMemory
	};

	struct DataHeader
	{
		void* Base;
		usize Alignment;
		usize Size;
		bool Tracked;
		bool Trackable;
		MemoryState State;
		u16 Category;

		u8 Padding[3];
	};

	DataHeader* GetHeader(void* ptr) noexcept;
	void SetMemoryState(void* ptr, MemoryState state) noexcept;
	usize GetAlignment(void* ptr) noexcept;
	usize GetSize(void* ptr) noexcept;
	void* Allocate(usize size, usize alignment = DefaultAlignment) noexcept;
	void Free(void* ptr) noexcept;
	void* Reallocate(void* ptr, usize newSize, usize alignment = 0) noexcept;
}

#pragma warning(push)
#ifdef COMPILER_MSVC
    #pragma warning( disable : 4595 )
#endif
inline void* operator new[](usize size) /* terminates on fail */
{
	if (void* ptr = MC::Memory::Allocate(size)) return ptr;
	MC::Utility::Debugging::Terminate("Failed to allocate! Terminating...");
}
inline void* operator new(usize size) /* terminates on fail */
{
	if (void* ptr = MC::Memory::Allocate(size)) return ptr;
	MC::Utility::Debugging::Terminate("Failed to allocate! Terminating...");
}
inline void* operator new[](usize size, std::align_val_t alignment) /* terminates on fail */
{
	if (void* ptr = MC::Memory::Allocate(size, static_cast<usize>(alignment))) return ptr;
	MC::Utility::Debugging::Terminate("Failed to allocate! Terminating...");
}
inline void* operator new(usize size, std::align_val_t alignment) /* terminates on fail */
{
	if (void* ptr = MC::Memory::Allocate(size, static_cast<usize>(alignment))) return ptr;
	MC::Utility::Debugging::Terminate("Failed to allocate! Terminating...");
}
inline void* operator new[](usize size, const std::nothrow_t&) noexcept
{
	return MC::Memory::Allocate(size, alignof(MC::MaxAlignT));
}
inline void* operator new(usize size, const std::nothrow_t&) noexcept
{
	return MC::Memory::Allocate(size, alignof(MC::MaxAlignT));
}
inline void* operator new[](usize size, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
	return MC::Memory::Allocate(size, static_cast<usize>(alignment));
}
inline void* operator new(usize size, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
	return MC::Memory::Allocate(size, static_cast<usize>(alignment));
}

inline void operator delete[](void* ptr) noexcept
{
	MC::Memory::Free(ptr);
}
inline void operator delete(void* ptr) noexcept
{
	return MC::Memory::Free(ptr);
}
inline void operator delete[](void* ptr, std::align_val_t) noexcept
{
	MC::Memory::Free(ptr);
}
inline void operator delete(void* ptr, std::align_val_t) noexcept
{
	return MC::Memory::Free(ptr);
}
#pragma warning(pop)