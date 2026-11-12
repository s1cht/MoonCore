#pragma once

// TODO: Write futures
#include <future>

namespace MC::Async
{
    template<typename T>
    using Future = std::future<T>;
}