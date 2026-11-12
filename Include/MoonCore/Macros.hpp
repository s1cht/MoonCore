#pragma once
#include "MoonCore/Platform/Base/PlatformMacros.hpp"

// Bitwise operations
#define BIT(x) static_cast<u64>(1 << (x))
#define IS_POWER_OF_2(x) x && !((x) & ((x) - 1))

// Attributes
#define MC_DEPRECATED [[deprecated]]
#define MC_DEPRECATED_MSG(text) [[deprecated(text)]]

#define MC_NODISCARD [[nodiscard]]
#define MC_NODISCARD_MSG(text) [[nodiscard(text)]]

#define MC_MAYBE_UNUSED [[maybe_unused]]
#define MC_NORETURN [[noreturn]]

#define MC_GB(bytes) static_cast<usize>((bytes) * BIT(30))
#define MC_MB(bytes) static_cast<usize>((bytes) * BIT(20))
#define MC_KB(bytes) static_cast<usize>((bytes) * BIT(10))