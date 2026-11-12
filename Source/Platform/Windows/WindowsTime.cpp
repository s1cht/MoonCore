#include "WindowsTime.hpp"
#include "Platform/Windows/WindowsInclude.hpp"

namespace MC::Clock
{
	u64 WindowsTimeSource::GetTick() const noexcept
	{
		LARGE_INTEGER ticks;
		QueryPerformanceCounter(&ticks);
		return static_cast<u64>(ticks.QuadPart);
	}

	u64 WindowsTimeSource::GetFrequency() const noexcept
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		return static_cast<u64>(frequency.QuadPart);
	}

    Date WindowsTimeSource::ConvertTickToDate(u64 tick) const noexcept
    {
		FILETIME fileTime;
		fileTime.dwLowDateTime = static_cast<DWORD>(tick & 0xFFFFFFFF);
		fileTime.dwHighDateTime = static_cast<DWORD>(tick >> 32);

		SYSTEMTIME systemTime;

		if (FileTimeToSystemTime(&fileTime, &systemTime))
		{
			Date result;
			result.Year = static_cast<u16>(systemTime.wYear);
			result.Month = static_cast<u8>(systemTime.wMonth);
			result.DayOfWeek = static_cast<u8>(systemTime.wDayOfWeek);
			result.Day = static_cast<u8>(systemTime.wDay);
			result.Hour = static_cast<u8>(systemTime.wHour);
			result.Minute = static_cast<u8>(systemTime.wMinute);
			result.Second = static_cast<u8>(systemTime.wSecond);
			result.Milliseconds = static_cast<u16>(systemTime.wMilliseconds);

			return result;
		}

	    return Date{};
    }

    Date WindowsTimeSource::GetDate() const noexcept
    {
		FILETIME fileTime;
		FILETIME localFileTime;
		GetSystemTimePreciseAsFileTime(&fileTime);
		
		if (!FileTimeToLocalFileTime(&fileTime, &localFileTime))
			return Date{};

		SYSTEMTIME systemTime;
		if (!FileTimeToSystemTime(&localFileTime, &systemTime))
			return Date{};

		Date result;
		result.Year = static_cast<u16>(systemTime.wYear);
		result.Month = static_cast<u8>(systemTime.wMonth);
		result.DayOfWeek = static_cast<u8>(systemTime.wDayOfWeek);
		result.Day = static_cast<u8>(systemTime.wDay);
		result.Hour = static_cast<u8>(systemTime.wHour);
		result.Minute = static_cast<u8>(systemTime.wMinute);
		result.Second = static_cast<u8>(systemTime.wSecond);
		result.Milliseconds = static_cast<u16>(systemTime.wMilliseconds);
		return result;
    }

    Memory::Scope<TimeSource> TimeSource::Create() noexcept
	{
		return Memory::Scope<TimeSource>(new(std::nothrow) WindowsTimeSource());
	}
}