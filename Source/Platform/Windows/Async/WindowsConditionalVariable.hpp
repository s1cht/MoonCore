#pragma once
#include "MoonCore/Async/ConditionVariable.hpp"
#include "Platform/Windows/WindowsInclude.hpp"

namespace MC::Windows::Async
{
    class WindowsConditionVariable final: public MC::Async::ConditionVariableBase
    {
    public:
        WindowsConditionVariable() noexcept;
        WindowsConditionVariable(const WindowsConditionVariable&) = delete;

        ~WindowsConditionVariable() noexcept override = default;

    public:
        WindowsConditionVariable& operator=(const WindowsConditionVariable&) = delete;

    public:
        void NotifyOne() noexcept override;
        void NotifyAll() noexcept override;
        void Wait(MC::Async::Mutex* mutex) noexcept override;
        bool Wait(MC::Async::Mutex* mutex, Clock::Timestep duration) noexcept override;
    
    private:
        CONDITION_VARIABLE m_Variable;
    };
}
