#pragma once
#include "MoonCore/Core.hpp"

#define MC_UNREACHABLE()

#ifdef COMPILER_MSVC
#   undef MC_UNREACHABLE
#   define MC_UNREACHABLE() __assume(false)
#elif defined(COMPILER_CLANG) || defined(COMPILER_GNUC)
#   undef MC_UNREACHABLE
#   define MC_UNREACHABLE() __builtin_unreachable()
#endif

namespace MC
{
    constexpr bool PartialDebuggerEnabled() noexcept
    {
#ifdef MC_DIST
        return false;
#else
        return true;
#endif
    }

    constexpr bool DebuggerEnabled() noexcept
    {
#ifdef MC_DEBUG
        return true;
#else
        return false;
#endif
    }
}

namespace MC::Utility::Debugging
{
    struct SourceData
    {
        consteval SourceData(const char7* const fileName = __builtin_FILE(), const char7* const function = __builtin_FUNCSIG(),
            const u32 line = __builtin_LINE(), const u32 column = __builtin_COLUMN()) noexcept
        {
            Line = line;
            Column = column;
            File = fileName;
            Function = function;
        }

        constexpr bool Valid() const noexcept { return Line == ~0u || Column == ~0u || File == "" || Function == ""; }

        u32 Line;
        u32 Column;
        const char7* File;
        const char7* Function;
    };

    static constexpr SourceData EmptySource = SourceData("", "", ~0u, ~0u);

    void Log(const char7* message) noexcept;

#pragma warning(push)
#pragma warning(disable : 4645)
    MC_NORETURN constexpr static void Terminate(const char7* message) noexcept
    {
        if consteval
        {
            MC_UNREACHABLE();
        }

        Log(message);
        abort();
    }

    MC_NORETURN constexpr static void Terminate() noexcept
    {
        if consteval
        {
            MC_UNREACHABLE();
        }

        abort();
    }
#pragma warning(pop)
}

#define MC_TERMINATE_IF_NOT(expression)                  \
    do {                                                 \
        if (!(expression)) [[unlikely]] {                \
            ::MC::Utility::Debugging::Terminate(); \
        }                                                \
    } while(0)

#define MC_TERMINATE_IF_NOT_MSG(expression, message)            \
    do {                                                        \
        if (!(expression)) [[unlikely]] {                       \
            ::MC::Utility::Debugging::Terminate(message); \
        }                                                       \
    } while(0)

// Sanity check for containers (and other Core objects) checks, does nothing when Distribute build is active
// When expression is false, result depends on the context
// 1: On runtime - MC::Utility::Debugging::Terminate() is called 
// 2: On constant evaluated - Invokes undefined behavior
// ATTENTION: Use this macro only with comparing expressions, in Distribute this macro can't invoke anything in expression
#define MC_CHECK(expression, message)
// Validates expression
// When expression is false, result depends on the context
// 1: On runtime - MC::Utility::Debugging::Terminate() is called 
// 2: On constant evaluated - Invokes undefined behavior
#define MC_VALIDATE(expression, message)
// Validates expression and formats log message on failure
// When expression is false, result depends on the context
// 1: On runtime - MC::Utility::Debugging::Terminate() is called 
// 2: On constant evaluated - Invokes undefined behavior
#define MC_RUNTIME_VALIDATE(expression, formatExpression, ...)

#if defined(MC_DEBUG)

#else
#endif

#if defined(MC_DEBUG) || defined(MC_RELEASE)
#   undef MC_ASSERT
#   undef MC_CHECK
#   undef MC_VALIDATE
#   undef MC_RUNTIME_VALIDATE
#   define MC_ASSERT(expression, ...)	                   \
		do {								               \
			if (!(expression)) [[unlikely]] {              \
				::MC::Logging::Logger::Fatal(__VA_ARGS__); \
				__debugbreak();				               \
			}								               \
		} while (0)
#   define MC_CHECK(expression, message) MC_TERMINATE_IF_NOT_MSG(expression, message)  
#   define MC_VALIDATE(expression, message) MC_TERMINATE_IF_NOT_MSG(expression, message)
#   define MC_RUNTIME_VALIDATE(expression, formatExpression, ...) MC_TERMINATE_IF_NOT_MSG(expression, formatExpression)
#else
#   undef MC_ASSERT
#   undef MC_CHECK
#   undef MC_VALIDATE
#   undef MC_RUNTIME_VALIDATE
#   define MC_ASSERT(expression, type, ...)
#   define MC_CHECK(expression, message)             
#   define MC_VALIDATE(expression, message)                       MC_TERMINATE_IF_NOT(expression)  
#   define MC_RUNTIME_VALIDATE(expression, formatExpression, ...) MC_TERMINATE_IF_NOT(expression)
#endif