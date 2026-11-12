#pragma once
#include "MoonCore/Types.hpp"

namespace MC::Memory
{
	template <typename T>
	class HeapAllocator
	{
	public:
		using ValueType = T;

	public:
		static ValueType* Allocate(usize count = 1) noexcept
		{
			void* ptr = ::operator new(count * sizeof(ValueType), std::align_val_t{ alignof(ValueType) }, std::nothrow);
			return static_cast<T*>(ptr);
		}

		static void Deallocate(ValueType* ptr, usize count = 1) noexcept
		{
			::operator delete(ptr, count * sizeof(ValueType));
		}
	};
}