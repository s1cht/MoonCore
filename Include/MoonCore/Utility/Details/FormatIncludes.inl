#pragma once
#define FMT_USE_EXCEPTIONS 0
#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <fmt/xchar.h>
#include <fmt/color.h>
namespace MC::External
{
    namespace fmt = ::fmt;
}