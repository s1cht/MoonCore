#pragma once
#include "MoonCore/Clock/Time.hpp"
#include "MoonCore/Logging/Logger.hpp"

namespace MC::Profiling
{
    // TODO: Add future-like benchmark result structure for profiler output

    template<Concept::TimeUnit Unit, usize OutputPrecision = 4>
    class Benchmark
    {
    public:
        Benchmark(const MC::StringView& benchmarkName) noexcept
            : m_StartTime(Clock::Time::Now()), m_BenchmarkName(benchmarkName) 
        {}

        ~Benchmark() noexcept
        {
            Clock::Timestep duration = Clock::Time::Now() - m_StartTime;
            MC_LOG(Profiling, Logging::LogLevel::Info, TEXT("Benchmark '%s' ended lasting %.5f%s"), 
                m_BenchmarkName, Unit::AsUnit(duration), Unit::UnitName);
        }

    private:
        Clock::Timepoint m_StartTime;
        MC::StringView m_BenchmarkName;
    };
}