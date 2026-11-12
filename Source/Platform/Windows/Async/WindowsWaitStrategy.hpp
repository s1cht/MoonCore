#pragma once
#include "MoonCore/Async/WaitStrategy.hpp"

namespace MC::Async
{
    class WindowsWaitStrategy : public WaitStrategy
    {
    public:
        bool WaitImpl(volatile void* waitable, void* comparable, usize size, u32 timeout) noexcept  override;
        void NotifyImpl(void* waitable) noexcept override;
        void NotifyAllImpl(void* waitable) noexcept override;
    };
}