#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Async/Atomic.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "MoonCore/Utility/Format.hpp"

namespace MC::Logging
{
	struct FormatString
	{
		MC::StringView fmt;
		MC::Utility::Debugging::SourceData source;

		template<typename T>
		FormatString(const T& str, const MC::Utility::Debugging::SourceData& src = MC::Utility::Debugging::EmptySource) noexcept : fmt(str), source(src) {}
	};

	class Logger
	{
	public:
		struct Data;

	    enum class Module : u8
		{
			Core = 0,
			Client,
			Graphics,
			Memory,
			Profiling,
		};

		enum class Severity
		{
		    Trace, Debug, Info, Warn, Error, Fatal
		};

	public:
		Logger() noexcept : m_Data(nullptr), m_Alive(false) {}

	public:
		void Initialize() noexcept;
		void InitializeFileLogging(const MC::StringView& logDirectory) noexcept;
		void Shutdown() noexcept;

	public:
		static Logger& Get() noexcept
		{
			static Logger instance;
			return instance;
		}

		template<typename... ArgTypes>
	    static void Trace(Module type, const FormatString& fmt, ArgTypes&&... args) noexcept
		{
			const String formatted = FormatMessage(fmt.fmt, Forward<ArgTypes>(args)...);
			OutputFormatted(type, Severity::Trace, fmt.source, formatted);
		}

		template<typename... ArgTypes>
		static void Info(Module type, const FormatString& fmt, ArgTypes&&... args) noexcept
		{
			const String formatted = FormatMessage(fmt.fmt, Forward<ArgTypes>(args)...);
			OutputFormatted(type, Severity::Trace, fmt.source, formatted);
		}

		template<typename... ArgTypes>
		static void Warn(Module type, const FormatString& fmt, ArgTypes&&... args) noexcept
		{
			const String formatted = FormatMessage(fmt.fmt, Forward<ArgTypes>(args)...);
			OutputFormatted(type, Severity::Warn, fmt.source, formatted);
		}

		template<typename... ArgTypes>
		static void Error(Module type, const FormatString& fmt, ArgTypes&&... args) noexcept
		{
			const String formatted = FormatMessage(fmt.fmt, Forward<ArgTypes>(args)...);
			OutputFormatted(type, Severity::Error, fmt.source, formatted);
		}

		template<typename... ArgTypes>
		static void Fatal(Module type, const FormatString& fmt, ArgTypes&&... args) noexcept
		{
			const String formatted = FormatMessage(fmt.fmt, Forward<ArgTypes>(args)...);
			OutputFormatted(type, Severity::Fatal, fmt.source, formatted);
		}

		template<typename... ArgTypes>
		static void Debug(Module type, Severity severity, const FormatString& fmt, ArgTypes&&... args) noexcept
		{
#		if defined(MC_DEBUG)
			String formatted = FormatMessage(fmt.fmt, Forward<ArgTypes>(args)...);
			formatted = FormatColor(severity, formatted);
			OutputFormatted(type, severity, fmt.source, formatted);
#		endif
		}

		static void InitializeLogger() noexcept;
		static void InitializeFileLogger(const MC::StringView& logDirectory) noexcept;
		static void ShutdownLogger() noexcept;

	private:
		static String FormatColor(Severity color, const MC::StringView& message) noexcept;
		static void OutputFormatted(Module type, Severity severity, const MC::Utility::Debugging::SourceData& source, const MC::StringView& message) noexcept;
		template<typename... ArgTypes>
		static String FormatMessage(const MC::StringView& fmt, ArgTypes&&... args) noexcept
		{
			String formatted = fmt.ToString();
			Formatter::FormatTo(formatted, fmt, Forward<ArgTypes>(args)...);
			return formatted;
		}

	private:
		Data* m_Data;
		Async::AtomicBool m_Alive;
	};
}

#undef LoggerBase
#undef LoggerImpl