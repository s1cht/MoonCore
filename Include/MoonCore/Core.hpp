#pragma once
// TODO: Add Linux support
#include "MoonCore/Types.hpp"
#include "MoonCore/Macros.hpp"
#include "MoonCore/TypeTraits.hpp"
#include "MoonCore/Concept.hpp"
#include <stdlib.h>
#include <string.h>

#define _CRT_SECURE_NO_WARNINGS
#define COREAPI
#if defined(PLATFORM_WINDOWS)
#   if !defined(MOON_CORE_STATIC_LIB)
#       undef COREAPI
#	    ifdef MOON_CORE_LIBRARY_BUILD
#	    	define COREAPI LIBRARY_EXPORT
#	    else	
#	    	define COREAPI LIBRARY_IMPORT
#	    endif 
#   endif
//#elif defined(PLATFORM_LINUX)
#else
#	error "Moon Engine currently supports only Windows!"
#endif

namespace MC
{
    template <typename ReturnType = u64>
    static constexpr ReturnType Bit(u8 bit) noexcept { return static_cast<ReturnType>(1ull << bit); }
}