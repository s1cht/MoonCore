#pragma once
#include "MoonCore/Types.hpp"

// I must use it, that is a C++ support header
#include <initializer_list>

namespace MC::Utility
{
    template<typename T> using InitializerList = std::initializer_list<T>;
}