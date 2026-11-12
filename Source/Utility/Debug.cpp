#include "MoonCore/Utility/Debug.hpp"
#include "MoonCore/Logging/Logger.hpp"

using namespace MC::Logging;

namespace MC::Utility::Debugging
{
    void Log(const char7* message) noexcept
    {
        Logger::Fatal(Logger::Module::Core, message);
    }
}
