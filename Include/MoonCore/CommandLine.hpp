#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Containers/Array.hpp"
#include "MoonCore/Containers/String.hpp"

namespace MC
{
    class CommandLine
    {
    public:
        static MC::StringView Get() noexcept;
        static const Array<String>& GetArguments() noexcept;
        static MC::StringView GetProgramPath() noexcept;
        static bool Initialize(i32 argumentCount, const char7** arguments) noexcept;

    private:
        static Array<MC::String> s_Arguments;
        static MC::String s_CommandLine;
        static MC::String s_ProgramPath;
    };
}