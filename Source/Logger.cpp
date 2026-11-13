#include "MoonCore/Logging/Logger.hpp"
#include "MoonCore/Async/Mutex.hpp"
#include "MoonCore/Containers/Array.hpp"
#include "MoonCore/Clock/Time.hpp"

#undef TEXT
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/dup_filter_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <filesystem>

#undef TEXT
#define TEXT(x) TEXT_HELPER(x)

using namespace spdlog;

namespace MC::Logging
{
    class PreFileOutputSink : public sinks::base_sink<std::mutex>
    {
    public:
        PreFileOutputSink() noexcept { m_Messages.Reserve(100); }
        ~PreFileOutputSink() noexcept override = default;

        const MC::Array<details::log_msg>& GetMessages() const noexcept { return m_Messages; }

    protected:
        void sink_it_(const details::log_msg& msg) override
        {
            m_Messages.EmplaceBack(msg);
        }

        void flush_() override
        {
            m_Messages.Clear();
        }

    private:
        MC::Array<details::log_msg> m_Messages;
    };

    constexpr level::level_enum MCLevelToSpdlog(Logger::Severity level) noexcept
    {
        switch (level)
        {
            case Logger::Severity::Trace: return level::trace;
            case Logger::Severity::Debug: return level::debug;
            case Logger::Severity::Info: return level::info;
            case Logger::Severity::Warn: return level::warn;
            case Logger::Severity::Error: return level::err;
            case Logger::Severity::Fatal: return level::critical;
        }
        return level::off;
    }

    constexpr usize QueueSize = 256;

    struct Logger::Data
    {
        // Thread pool
        std::shared_ptr<details::thread_pool> ThreadPool;

        // Sinks
        std::shared_ptr<sinks::dup_filter_sink_mt> FilterSink;
        std::shared_ptr<sinks::stdout_color_sink_st> ConsoleSink;
        std::shared_ptr<sinks::basic_file_sink_st> FileSink;
        std::shared_ptr<PreFileOutputSink> TemporarySink;

        std::shared_ptr<logger> Core;
        std::shared_ptr<logger> Client;
        std::shared_ptr<logger> Graphics;
        std::shared_ptr<logger> Memory;
        std::shared_ptr<logger> Profiling;
    };

    inline std::shared_ptr<sinks::basic_file_sink_st> TryCreateFileSink(const std::string& logFile) noexcept
    {
        std::filesystem::path path(logFile);
        std::error_code ec;

        std::filesystem::create_directories(path.parent_path(), ec);
        if (ec) return nullptr;

        {
            std::ofstream test(logFile, std::ios::app);
            if (!test.is_open()) return nullptr;
        }

        return std::make_shared<sinks::basic_file_sink_st>(logFile);
    }

    void Logger::Initialize() noexcept
    {
        m_Alive.store(true);
#ifndef MC_DEBUG
        set_pattern("%^[%H:%M:%S:%e ThreadID: %t] %n: %$ %v");
#else
        set_pattern("%^[%H:%M:%S:%e ThreadID: %t] [%@] %n: %$ %v");
#endif

        m_Data = new(std::nothrow) Data();

        m_Data->ThreadPool = std::make_shared<details::thread_pool>(QueueSize, 1);

        m_Data->FilterSink = std::make_shared<sinks::dup_filter_sink_mt>(std::chrono::seconds(2));
        m_Data->ConsoleSink = std::make_shared<sinks::stdout_color_sink_st>();
        m_Data->TemporarySink = std::make_shared<PreFileOutputSink>();

        m_Data->FilterSink->add_sink(m_Data->ConsoleSink);
        m_Data->FilterSink->add_sink(m_Data->TemporarySink);

        m_Data->Core = std::make_shared<async_logger>("Core", m_Data->FilterSink, m_Data->ThreadPool, async_overflow_policy::overrun_oldest);
        m_Data->Client = std::make_shared<async_logger>("Client", m_Data->FilterSink, m_Data->ThreadPool, async_overflow_policy::overrun_oldest);
        m_Data->Graphics = std::make_shared<async_logger>("Render", m_Data->FilterSink, m_Data->ThreadPool, async_overflow_policy::overrun_oldest);
        m_Data->Memory = std::make_shared<async_logger>("HeapMemory", m_Data->FilterSink, m_Data->ThreadPool, async_overflow_policy::overrun_oldest);
        m_Data->Profiling = std::make_shared<async_logger>("Profiling", m_Data->FilterSink, m_Data->ThreadPool, async_overflow_policy::overrun_oldest);

        m_Data->Graphics->set_pattern("%^[%H:%M:%S] %n: %$ %v");

        auto SafeErrorHandler = [](const std::string& msg)
            {
                fmt::println(stderr, "[Logger Error] %s\n", msg.c_str());
            };

        m_Data->Core->set_error_handler(SafeErrorHandler);
        m_Data->Client->set_error_handler(SafeErrorHandler);
        m_Data->Graphics->set_error_handler(SafeErrorHandler);
        m_Data->Memory->set_error_handler(SafeErrorHandler);
        m_Data->Profiling->set_error_handler(SafeErrorHandler);

#ifdef MC_DEBUG
        m_Data->Core->set_level(level::level_enum::trace);
        m_Data->Client->set_level(level::level_enum::trace);
        m_Data->Graphics->set_level(level::level_enum::trace);
        m_Data->Memory->set_level(level::level_enum::trace);
        m_Data->Profiling->set_level(level::level_enum::trace);
#else
        m_Data->Core->set_level(level::info);
        m_Data->Client->set_level(level::info);
        m_Data->Graphics->set_level(level::info);
        m_Data->Memory->set_level(level::info);
        m_Data->Profiling->set_level(level::info);
#endif
    }

    void Logger::InitializeFileLogging(const MC::StringView& logDirectory) noexcept
    {
        if (!m_Data) return;

        Clock::Date date = Clock::Time::Now().Time();
        std::string logFile = std::string(AsChar7(logDirectory.String())) +
            fmt::format("Log_{}{}{}_{}{}{}{}.mlog",
                date.Day, date.Month, date.Year, date.Hour, date.Minute, date.Second, date.Milliseconds);
        m_Data->FileSink = TryCreateFileSink(logFile);
        m_Data->FilterSink->remove_sink(m_Data->TemporarySink);

        const auto& messages = m_Data->TemporarySink->GetMessages();
        for (auto message : messages)
            m_Data->FileSink->log(message);

        m_Data->TemporarySink->flush();
        m_Data->TemporarySink.reset();

        if (m_Data->FileSink) m_Data->FilterSink->add_sink(m_Data->FileSink);
    }

    void Logger::Shutdown() noexcept
    {
        if (!m_Data) return;

        m_Data->Core->flush();
        m_Data->Core.reset();

        m_Data->Client->flush();
        m_Data->Client.reset();

        m_Data->Graphics->flush();
        m_Data->Graphics.reset();

        m_Data->Memory->flush();
        m_Data->Memory.reset();

        m_Data->Profiling->flush();
        m_Data->Profiling.reset();

        m_Data->FilterSink->flush();
        m_Data->FilterSink.reset();

        m_Data->ConsoleSink->flush();
        m_Data->ConsoleSink.reset();

        if (m_Data->FileSink)
        {
            m_Data->FileSink->flush();
            m_Data->FileSink.reset();
        }

        m_Data->ThreadPool.reset();
        m_Alive.store(false);
        delete m_Data;
    }

    void Logger::InitializeLogger() noexcept
    {
        Get().Initialize();
    }

    void Logger::InitializeFileLogger(const StringView& logDirectory) noexcept
    {
        Get().InitializeFileLogging(logDirectory);
    }

    void Logger::ShutdownLogger() noexcept
    {
        Get().Shutdown();
    }

    String Logger::FormatColor(Severity color, const MC::StringView& message) noexcept
    {
        fmt::text_style style;
        switch (color)
        {
            case Severity::Trace: style = fmt::fg(fmt::terminal_color::bright_black) | fmt::emphasis::italic; break;
            case Severity::Debug: style = fmt::fg(fmt::terminal_color::white) | fmt::emphasis::italic; break;
            case Severity::Info:  style = fmt::fg(fmt::terminal_color::white) | fmt::emphasis::italic; break;
            case Severity::Warn:  style = fmt::fg(fmt::terminal_color::yellow) | fmt::emphasis::italic; break;
            case Severity::Error: style = fmt::fg(fmt::terminal_color::red) | fmt::emphasis::italic; break;
            case Severity::Fatal: style = fmt::fg(fmt::terminal_color::bright_red) | fmt::emphasis::italic; break;
        }

        return Formatter::Format("{}", fmt::styled(message, style));
    }

    void Logger::OutputFormatted(Module type, Severity severity, const Utility::Debugging::SourceData& source,
                                 const StringView& message) noexcept
    {
        if (!Get().m_Alive.load()) return;
        Data* data = Get().m_Data;
        std::string_view stdMessage = std::string_view(AsChar7(message.String()), message.Size());

        if (source.Valid())
        {
            switch (type)
            {
                case Module::Core:      return data->Core->log(MCLevelToSpdlog(severity), stdMessage);
                case Module::Client:    return data->Client->log(MCLevelToSpdlog(severity), stdMessage);
                case Module::Graphics:  return data->Graphics->log(MCLevelToSpdlog(severity), stdMessage);
                case Module::Memory:    return data->Memory->log(MCLevelToSpdlog(severity), stdMessage);
                case Module::Profiling: return data->Profiling->log(MCLevelToSpdlog(severity), stdMessage);
            }
        }

        source_loc src;
        src.line = static_cast<i32>(source.Line);
        src.filename = source.File;
        src.funcname = source.Function;

        switch (type)
        {
            case Module::Core:      return data->Core->log(src, MCLevelToSpdlog(severity), stdMessage);
            case Module::Client:    return data->Client->log(src, MCLevelToSpdlog(severity), stdMessage);
            case Module::Graphics:  return data->Graphics->log(src, MCLevelToSpdlog(severity), stdMessage);
            case Module::Memory:    return data->Memory->log(src, MCLevelToSpdlog(severity), stdMessage);
            case Module::Profiling: return data->Profiling->log(src, MCLevelToSpdlog(severity), stdMessage);
        }
    }
}