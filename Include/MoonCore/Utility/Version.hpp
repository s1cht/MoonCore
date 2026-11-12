#pragma once
#include "MoonCore/Types.hpp"
#include "Format.hpp"

namespace MC
{
    struct Version
    {
        u16 Major = 0;
        u16 Minor = 0;
        u32 Patch = 0;
    };
}

MC_FMT_FORMATTER(MC::Version, "%d.%d.%d", MC_FMT_FORMATTER_MEMBER(Major), MC_FMT_FORMATTER_MEMBER(Minor), MC_FMT_FORMATTER_MEMBER(Patch));