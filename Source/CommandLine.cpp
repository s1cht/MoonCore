#include "MoonCore/CommandLine.hpp"

namespace MC
{
    Array<MC::String> CommandLine::s_Arguments = {};
    String CommandLine::s_CommandLine = TEXT("");
    String CommandLine::s_ProgramPath = TEXT("");

    StringView CommandLine::Get() noexcept
    {
        return s_CommandLine;
    }

    const Array<String>& CommandLine::GetArguments() noexcept
    {
        return s_Arguments;
    }

    MC::StringView CommandLine::GetProgramPath() noexcept
    {
        return s_ProgramPath;
    }

    bool CommandLine::Initialize(i32 argumentCount, const char7** arguments) noexcept
    {
        if (argumentCount <= 0) return false;

        s_ProgramPath = AsChar8(arguments[0]);
        s_Arguments.Reserve(argumentCount);

        for (i32 i = 1; i < argumentCount; ++i)
        {
            s_CommandLine += AsChar8(arguments[i]);
            s_Arguments.EmplaceBack(AsChar8(arguments[i]));
        }

        return true;
    }
}
