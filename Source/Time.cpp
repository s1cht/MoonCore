#include "MoonCore/Clock/Time.hpp"

namespace MC::Clock
{
#ifndef PLATFORM_WINDOWS
	Memory::Scope<TimeSource> TimeSource::Create() noexcept { return nullptr; }
#endif

    Date Timepoint::Time() const noexcept
	{
		return Time::TickToTime(m_Tick);
	}

	Date Timestep::Time() const noexcept
    {
		return Time::TickToTime(m_Ticks);
    }

    f64 Timestep::AsNanoseconds() const noexcept
	{
		return (static_cast<f64>(m_Ticks) / static_cast<f64>(Time::GetFrequency())) * 1e9;
	}
	f64 Timestep::AsMicroseconds() const noexcept
	{
		return (static_cast<f64>(m_Ticks) / static_cast<f64>(Time::GetFrequency())) * 1e6;
	}
	f64 Timestep::AsMilliseconds() const noexcept
	{
		return (static_cast<f64>(m_Ticks) / static_cast<f64>(Time::GetFrequency())) * 1e3;
	}
	f64 Timestep::AsSeconds() const noexcept
	{
		return static_cast<f64>(m_Ticks) / static_cast<f64>(Time::GetFrequency());
	}
	
	void Time::InitializeImpl() noexcept
	{
		m_Source = TimeSource::Create();
		m_Frame.LastFrame = Now();
		m_Frame.DeltaTime = Timestep(0);
		m_Logic.LastFrame = Now();
		m_Logic.DeltaTime = Timestep(0);

		m_PhysicsFixedTime = Timestep(static_cast<u64>(static_cast<f64>(m_Source->GetFrequency()) / 60.0));
	}

	void Time::ShutdownImpl() noexcept
	{
		m_Source.Reset();
	}

	Timestep Time::UpdateImpl() noexcept
	{
		Timepoint current = Now();
		m_Frame.DeltaTime = Timestep(m_Frame.LastFrame, current);
		m_Frame.LastFrame = current;
		return m_Frame.DeltaTime;
	}

	Timestep Time::LogicStepImpl() noexcept
	{
		Timepoint current = Now();
		m_Logic.DeltaTime = Timestep(m_Logic.LastFrame, current);
		m_Logic.LastFrame = current;
		return m_Logic.DeltaTime;
	}

	Date Time::TickToTimeImpl(u64 tick) const noexcept
	{
		return m_Source->ConvertTickToDate(tick);
	}

    Date Time::GetDateImpl() const noexcept
    {
		return m_Source->GetDate();
    }
}
