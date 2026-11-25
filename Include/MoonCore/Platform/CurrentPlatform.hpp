#pragma once
#include "MoonCore/Types.hpp"

#if defined (COMPILER_MSVC)
#   include <intrin.h>
#else
#   if defined(ARCHITECTURE_X64)
#      include <intrin_x86_64.h>
#   else
#      error Unsupported architecture
#   endif
#endif

namespace MC
{
    enum class Platform : u8
    {
        Windows,
        Linux
    };

#if defined(PLATFORM_WINDOWS)
    constexpr Platform CurrentPlatform = Platform::Windows;
#elif  defined(PLATFORM_LINUX)
    constexpr Platform CurrentPlatform = Platform::Linux;
#endif

    constexpr bool IsWindows = CurrentPlatform == Platform::Windows;
    constexpr bool IsLinux = CurrentPlatform == Platform::Linux;
}