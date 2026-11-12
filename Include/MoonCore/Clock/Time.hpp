#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Concept.hpp"
#include "MoonCore/Containers/String/StringView.hpp"
#include "MoonCore/Memory/Pointers/Scope.hpp"
#include "MoonCore/Utility/Debug.hpp"

namespace MC::Clock
{
	class Timestep;
	class Timepoint;

	namespace Constants
	{
		inline constexpr u64 Max = 0xFFFFFFFFFFFFFFFFULL / 2;

		namespace Values
		{
			inline constexpr f64 Nanosecond = 0.001;
			inline constexpr u64 Microsecond = 10;
			inline constexpr u64 Millisecond = Microsecond * 1000;
			inline constexpr u64 Second = Millisecond * 1000;
			inline constexpr u64 Minute = Second * 60;
			inline constexpr u64 Hour = Minute * 60;
			inline constexpr u64 Day = Hour * 24;
			inline constexpr u64 Week = Day * 7;
			inline constexpr u64 Year = Day * 365;
		}
	}

	struct Date
	{
		u16 Year;
		u8 Month;
		u8 DayOfWeek;
		u8 Day;
		u8 Hour;
		u8 Minute;
		u8 Second;
		u16 Milliseconds;
	};

	class TimeSource
	{
	public:
		virtual ~TimeSource() noexcept = default;
		virtual u64 GetTick() const noexcept = 0;
		virtual u64 GetFrequency() const noexcept = 0;
		virtual Date ConvertTickToDate(u64 tick) const noexcept = 0;
		virtual Date GetDate() const noexcept = 0;

	public:
		static Memory::Scope<TimeSource> Create() noexcept;
	};

	class Timestep
	{
	public:
		constexpr explicit Timestep() noexcept
			: m_Ticks(0)
		{}
		constexpr Timestep(u64 ticks) noexcept
			: m_Ticks(ticks)
		{}
		constexpr Timestep(const Timepoint& start, const Timepoint& end) noexcept;

		constexpr Timestep(const Timestep&) noexcept = default;
		constexpr Timestep(Timestep&&) noexcept = default;
		constexpr ~Timestep() noexcept = default;

		constexpr Timestep& operator=(const Timestep&) noexcept = default;
		constexpr Timestep& operator=(Timestep&&) noexcept = default;

	public:
		constexpr bool operator==(const Timestep& other) const noexcept { return m_Ticks == other.m_Ticks; }
		constexpr bool operator<(const Timestep& other) const noexcept { return m_Ticks < other.m_Ticks; }
		constexpr bool operator<(u64 other) const noexcept { return m_Ticks < other; }
		constexpr bool operator<=(const Timestep& other) const noexcept { return m_Ticks <= other.m_Ticks; }
		constexpr bool operator<=(u64 other) const noexcept { return m_Ticks <= other; }
		constexpr bool operator>(const Timestep& other) const noexcept { return m_Ticks > other.m_Ticks; }
		constexpr bool operator>(u64 other) const noexcept { return m_Ticks > other; }
		constexpr bool operator>=(const Timestep& other) const noexcept { return m_Ticks >= other.m_Ticks; }
		constexpr bool operator>=(u64 other) const noexcept { return m_Ticks >= other; }

		constexpr Timestep operator+(const Timestep& other) const noexcept { return m_Ticks + other.m_Ticks; }
		constexpr Timestep operator-(const Timestep& other) const noexcept
		{
			MC_CHECK(m_Ticks >= other.m_Ticks, "Timestep subtraction result is less than 0");
			return m_Ticks - other.m_Ticks;
		}
		constexpr Timestep& operator+=(const Timestep& other) noexcept
		{
			m_Ticks += other.m_Ticks;
			return *this;
		}
		constexpr Timestep& operator-=(const Timestep& other) noexcept
		{
			MC_CHECK(m_Ticks >= other.m_Ticks, "Timestep subtraction result is less than 0");
			m_Ticks -= other.m_Ticks;
			return *this;
		}

		MC_NODISCARD constexpr u64 Ticks() const noexcept { return m_Ticks; }

	public:
		f64 AsNanoseconds() const noexcept;
		f64 AsMicroseconds() const noexcept;
		f64 AsMilliseconds() const noexcept;
		f64 AsSeconds() const noexcept;

		Date Time() const noexcept;

    private:
		u64 m_Ticks;
	};

	class Timepoint
	{
	public:
		constexpr explicit Timepoint() noexcept : m_Tick(0) {}
		constexpr Timepoint(u64 ticks) noexcept 
			: m_Tick(ticks)
		{
			MC_CHECK(m_Tick < Constants::Max, "Tick limit exceed");
		}

		constexpr Timepoint(const Timepoint&) noexcept = default;
		constexpr Timepoint(Timepoint&&) noexcept = default;
		constexpr ~Timepoint() noexcept = default;

		constexpr Timepoint& operator=(const Timepoint&) noexcept = default;
		constexpr Timepoint& operator=(Timepoint&&) noexcept = default;
	
	public:
		constexpr bool operator==(const Timepoint& other) const noexcept { return m_Tick == other.m_Tick; }
		constexpr bool operator<(const Timepoint& other) const noexcept { return m_Tick < other.m_Tick; }
		constexpr bool operator<(u64 other) const noexcept { return m_Tick < other; }
		constexpr bool operator<=(const Timepoint& other) const noexcept { return m_Tick <= other.m_Tick; }
		constexpr bool operator<=(u64 other) const noexcept { return m_Tick <= other; }
		constexpr bool operator>(const Timepoint& other) const noexcept { return m_Tick > other.m_Tick; }
		constexpr bool operator>(u64 other) const noexcept { return m_Tick > other; }
		constexpr bool operator>=(const Timepoint& other) const noexcept { return m_Tick >= other.m_Tick; }
		constexpr bool operator>=(u64 other) const noexcept { return m_Tick >= other; }

		constexpr Timestep operator-(const Timepoint& other) const noexcept
		{
			MC_CHECK(m_Tick >= other.m_Tick, "Timepoint subtraction result is less than 0");
			return m_Tick - other.m_Tick;
		}
		constexpr Timepoint operator+(const Timestep& other) const noexcept
		{
			return m_Tick + other.Ticks();
		}
		constexpr Timepoint operator+(u64 other) const noexcept
		{
			return m_Tick + other;
		}
		constexpr Timepoint operator-(const Timestep& other) const noexcept
		{
			MC_CHECK(m_Tick >= other.Ticks(), "Timepoint subtraction result is less than 0");
			return m_Tick - other.Ticks();
		}
		constexpr Timepoint operator-(u64 other) const noexcept
		{
			MC_CHECK(m_Tick < other, "Timepoint subtraction result is less than 0");
			return m_Tick - other;
		}
		constexpr Timepoint& operator+=(const Timestep& other) noexcept
		{
			m_Tick += other.Ticks();
			return *this;
		}
		constexpr Timepoint& operator+=(u64 other) noexcept
		{
			m_Tick += other;
			return *this;
		}
		constexpr Timepoint& operator-=(const Timestep& other) noexcept
		{
			MC_CHECK(m_Tick >= other.Ticks(), "Timepoint subtraction result is less than 0");
			m_Tick -= other.Ticks();
			return *this;
		}
		constexpr Timepoint& operator-=(u64 other) noexcept
		{
			MC_CHECK(m_Tick < other, "Timepoint subtraction result is less than 0");
			m_Tick -= other;
			return *this;
		}

	public:
		MC_NODISCARD Date Time() const noexcept;
		MC_NODISCARD constexpr u64 Tick() const noexcept { return m_Tick; }

	private:
		u64 m_Tick;
	};

	struct FrameTime
	{
		Timepoint LastFrame;
		Timestep DeltaTime;
	};

	class Time
	{
	public:
		inline static Time& Get() noexcept
		{
			static Time instance;
			return instance;
		}

		static void Initialize() noexcept { Get().InitializeImpl(); }
		static void Shutdown() noexcept { Get().ShutdownImpl(); }
		
	    static Timestep Update() noexcept { return Get().UpdateImpl(); }
	    static Timestep LogicStep() noexcept { return Get().LogicStepImpl(); }
		
	    static Timestep GetFrameDeltaTime() noexcept { return Get().GetFrameDeltaTimeImpl(); }
		static Timestep GetPhysicsDeltaTime() noexcept { return Get().GetPhysicsDeltaTimeImpl(); }
		static Timestep GetLogicDeltaTime() noexcept { return Get().GetLogicDeltaTimeImpl(); }

	    static Date TickToTime(u64 tick) noexcept { return Get().TickToTimeImpl(tick); }
	    static Date GetDate() noexcept { return Get().GetDateImpl(); }
		static u64 GetFrequency() noexcept { return Get().m_Source->GetFrequency(); }

		static Timepoint Now() noexcept { return { Get().m_Source->GetTick() }; }

	private:
		void InitializeImpl() noexcept;
		void ShutdownImpl() noexcept;

		Timestep UpdateImpl() noexcept;
		Timestep LogicStepImpl() noexcept;
		
	    void SetPhysicsFixedTimeImpl(const Timestep& timestep) noexcept { m_PhysicsFixedTime = timestep; }

		Timestep GetFrameDeltaTimeImpl() const noexcept { return m_Frame.DeltaTime; }
		Timestep GetPhysicsDeltaTimeImpl() const noexcept { return m_PhysicsFixedTime; }
		Timestep GetLogicDeltaTimeImpl() const noexcept { return m_Logic.DeltaTime; }

		Date TickToTimeImpl(u64 tick) const noexcept;
		Date GetDateImpl() const noexcept;


	private:
		Memory::Scope<TimeSource> m_Source;

	private:
		FrameTime m_Frame;
		FrameTime m_Logic;
		Timestep m_PhysicsFixedTime;
	};

	namespace Unit
	{
		struct Nanosecond
		{
			static constexpr StringView UnitName = TEXT("ns");
			static f64 AsUnit(Timestep step) noexcept { return step.AsNanoseconds(); }
		};
		struct Microsecond
		{
			static constexpr StringView UnitName = TEXT("μs");
			static f64 AsUnit(Timestep step) noexcept { return step.AsMicroseconds(); }
		};
		struct Millisecond
		{
			static constexpr StringView UnitName = TEXT("ms");
			static f64 AsUnit(Timestep step) noexcept { return step.AsMilliseconds(); }
		};
		struct Second
		{
			static constexpr StringView UnitName = TEXT("s");
			static f64 AsUnit(Timestep step) noexcept { return step.AsSeconds(); }
		};
		struct Minute
		{
			static constexpr StringView UnitName = TEXT("min");
			static f64 AsUnit(Timestep step) noexcept { return step.AsSeconds() / 60.0; }
		};
	}

	constexpr Timestep::Timestep(const Timepoint& start, const Timepoint& end) noexcept
	{
		MC_CHECK(end.Tick() >= start.Tick(), "End must be >= start");
		m_Ticks = end.Tick() - start.Tick();
	}
}

namespace MC::Concept
{
	template<class T>
	concept TimeUnit = requires(MC::Clock::Timestep timestep)
	{
		SameAs<decltype(T::UnitName), StringView>;
		{ T::AsUnit(timestep) } -> ConvertibleTo<f64>;
	};
}