#pragma once
#include "MoonCore/Platform/Base/BaseTypes.hpp"
#include "MoonCore/Platform/Base/PlatformMacros.hpp"

// Windows platform types redefinition

struct WindowsPlatformTypes : BaseTypes
{
#ifdef COMPILER_MSVC
    typedef unsigned __int64 usize;
    typedef __int64 isize;
#elif defined(COMPILER_CLANG) || defined(COMPILER_GCC)
    typedef unsigned long long usize;
    typedef long long          isize;
#endif
};

typedef WindowsPlatformTypes PlatformTypes;