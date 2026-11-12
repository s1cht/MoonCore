#include "WindowsConditionalVariable.hpp"

#include "WindowsMutex.hpp"

namespace MC::Windows::Async
{
    WindowsConditionVariable::WindowsConditionVariable() noexcept
    {
        InitializeConditionVariable(&m_Variable);
    }

    void WindowsConditionVariable::NotifyOne() noexcept
    {
        WakeConditionVariable(&m_Variable);
    }

    void WindowsConditionVariable::NotifyAll() noexcept
    {
        WakeAllConditionVariable(&m_Variable);
    }

    void WindowsConditionVariable::Wait(MC::Async::Mutex* mutex) noexcept
    {
        SleepConditionVariableCS(&m_Variable, static_cast<WindowsMutex*>(mutex->Handle())->Handle(), INFINITE);
    }

    bool WindowsConditionVariable::Wait(MC::Async::Mutex* mutex, Clock::Timestep duration) noexcept
    {
        DWORD dwordDuration = static_cast<DWORD>(duration.AsMilliseconds());
        return SleepConditionVariableCS(&m_Variable, static_cast<WindowsMutex*>(mutex->Handle())->Handle(), dwordDuration) != FALSE;
    }
}

MC::Memory::Scope<MC::Async::ConditionVariableBase> MC::Async::ConditionVariableBase::Create()
{
    return Memory::MakeScope<MC::Windows::Async::WindowsConditionVariable>();
}