#pragma once
#include "MoonCore/Core.hpp"

#define MOON_UNREACHABLE()

#ifdef COMPILER_MSVC
#   undef MOON_UNREACHABLE
#   define MOON_UNREACHABLE() __assume(false)
#elif defined(COMPILER_CLANG) || defined(COMPILER_GNUC)
#   undef MOON_UNREACHABLE
#   define MOON_UNREACHABLE() __builtin_unreachable()
#endif

namespace MC
{
    constexpr bool PartialDebuggerEnabled() noexcept
    {
#ifdef MOON_DIST
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

    using TerminationHandler = void(*)();

    void COREAPI SetTerminationHandler(const TerminationHandler& handler) noexcept;
    TerminationHandler COREAPI GetTerminationHandler() noexcept;

    namespace Details
    {
        MOON_NORETURN COREAPI void TerminateRuntime(const char7* message, const SourceData& source) noexcept;
    }
#pragma warning(push)
#pragma warning(disable : 4645)
    MOON_NORETURN constexpr static void Terminate(const char7* message = nullptr, const SourceData& source = SourceData()) noexcept
    {
        if consteval
        {
            MOON_UNREACHABLE();
        }
        Details::TerminateRuntime(message, source);
    }
#pragma warning(pop)
}

#define MOON_TERMINATE_IF_NOT(expression)                  \
    do {                                                 \
        if (!(expression)) [[unlikely]] {                \
            ::MC::Utility::Debugging::Terminate(); \
        }                                                \
    } while(0)

#define MOON_TERMINATE_IF_NOT_MSG(expression, message)    \
    do {                                                  \
        if (!(expression)) [[unlikely]] {                 \
            ::MC::Utility::Debugging::Terminate(message); \
        }                                                 \
    } while(0)

// Sanity check for containers (and other Core objects) checks, does nothing when Distribute build is active
// When expression is false, result depends on the context
// 1: On runtime - MC::Utility::Debugging::Terminate() is called 
// 2: On constant evaluated - Invokes undefined behavior
// ATTENTION: Use this macro only with comparing expressions, in Distribute this macro can't invoke anything in expression
#define MOON_CHECK(expression, message)
// Validates expression
// When expression is false, result depends on the context
// 1: On runtime - MC::Utility::Debugging::Terminate() is called 
// 2: On constant evaluated - Invokes undefined behavior
#define MOON_VALIDATE(expression, message)

#if defined(MC_DEBUG) || defined(MC_RELEASE)
#   undef MOON_ASSERT
#   undef MOON_CHECK
#   undef MOON_VALIDATE
#   define MOON_ASSERT(expression, ...)	                   \
		do {								               \
			if (!(expression)) [[unlikely]] {              \
				::MC::Logging::Logger::Fatal(__VA_ARGS__); \
				__debugbreak();				               \
			}								               \
		} while (0)
#   define MOON_CHECK(expression, message) MOON_TERMINATE_IF_NOT_MSG(expression, message)  
#   define MOON_VALIDATE(expression, message) MOON_TERMINATE_IF_NOT_MSG(expression, message)
#else
#   undef MOON_ASSERT
#   undef MOON_CHECK
#   undef MOON_VALIDATE
#   define MOON_ASSERT(expression, type, ...)
#   define MOON_CHECK(expression, message)             
#   define MOON_VALIDATE(expression, message)                       MOON_TERMINATE_IF_NOT(expression)  
#endif