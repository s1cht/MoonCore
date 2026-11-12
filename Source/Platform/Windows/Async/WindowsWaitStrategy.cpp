#include "WindowsWaitStrategy.hpp"
#include "Platform/Windows/WindowsInclude.hpp"

namespace MC::Async
{
    bool WindowsWaitStrategy::WaitImpl(volatile void* waitable, void* comparable, usize size, u32 timeout) noexcept
    {
        return WaitOnAddress(waitable, comparable, size, timeout) != FALSE;
    }

    void WindowsWaitStrategy::NotifyImpl(void* waitable) noexcept
    {
        WakeByAddressSingle(waitable);
    }

    void WindowsWaitStrategy::NotifyAllImpl(void* waitable) noexcept
    {
        WakeByAddressAll(waitable);
    }

    void WaitStrategy::Initialize() noexcept
    {
        s_WaitStrategy.Reset(new(std::nothrow) WindowsWaitStrategy());
    }
}