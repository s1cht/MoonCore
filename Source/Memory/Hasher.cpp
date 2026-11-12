#include "MoonCore/Memory/Hasher.hpp"

#include <xxhash.h>

namespace MC::Memory
{
	usize Hash64(const void* input, usize length, u64 seed) noexcept
	{
		return static_cast<usize>(XXH64(input, length, seed));
	}
}
