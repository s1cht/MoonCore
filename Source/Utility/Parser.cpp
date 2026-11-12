#include "MoonCore/Utility/Parser.hpp"
#include "MoonCore/Logging/Logger.hpp"

using namespace MC::Logging;

namespace MC::Utility
{
    bool Parser::Parameter(const MC::StringView& line, const MC::StringView& match) noexcept
    {
        return line.FindFirst(match) != StringView::NoPosition;
    }

    bool Parser::Value(const MC::StringView& line, const MC::StringView& match, u64* value) noexcept
    {
        if (value == nullptr)
        {
            Logger::Error(Logger::Module::Core,
                "Can't parse line without value output!");
            return false;
        }

        usize position = line.FindFirst(match);
        if (position == StringView::NoPosition) return false;

        usize valuePosition;
        auto temp = Convert<StringView>::ToU64(line.Substring(position + match.Size()), &valuePosition);
        bool correct = true;
        bool seenDigit = false;
        for (usize i = 0; i < valuePosition; ++i)
        {
            char8 ch = line[position + match.Size() + i];
            if (ch == ' ') continue;
            if (ch == '-' && !seenDigit) { seenDigit = true; continue; }
            if (ch == '-') { correct = false; break; }
            seenDigit = true;
        }

        if (correct)
        {
            *value = temp;
            return true;
        }
        Logger::Error(Logger::Module::Core,
            Formatter::Format("Failed to parse value for an argument %s!", match));
        return false;
    }

    bool Parser::Value(const MC::StringView& line, const MC::StringView& match, u32* value) noexcept
    {
        if (value == nullptr)
        {
            Logger::Error(Logger::Module::Core,
                "Can't parse line without value output!");
            return false;
        }

        usize position = line.FindFirst(match);
        if (position == StringView::NoPosition) return false;

        usize valuePosition;
        auto temp = Convert<StringView>::ToU32(line.Substring(position + match.Size()), &valuePosition);
        bool correct = true;
        bool seenDigit = false;
        for (usize i = 0; i < valuePosition; ++i)
        {
            char8 ch = line[position + match.Size() + i];
            if (ch == ' ') continue;
            if (ch == '-' && !seenDigit) { seenDigit = true; continue; }
            if (ch == '-') { correct = false; break; }
            seenDigit = true;
        }

        if (correct)
        {
            *value = temp;
            return true;
        }
        Logger::Error(Logger::Module::Core,
            Formatter::Format("Failed to parse value for an argument %s!", match));
        return false;
    }

    bool Parser::Value(const MC::StringView& line, const MC::StringView& match, u16* value) noexcept
    {
        if (value == nullptr)
        {
            Logger::Error(Logger::Module::Core,
                "Can't parse line without value output!");
            return false;
        }

        usize position = line.FindFirst(match);
        if (position == StringView::NoPosition) return false;

        usize valuePosition;
        auto temp = Convert<StringView>::ToU16(line.Substring(position + match.Size()), &valuePosition);
        bool correct = true;
        bool seenDigit = false;
        for (usize i = 0; i < valuePosition; ++i)
        {
            char8 ch = line[position + match.Size() + i];
            if (ch == ' ') continue;
            if (ch == '-' && !seenDigit) { seenDigit = true; continue; }
            if (ch == '-') { correct = false; break; }
            seenDigit = true;
        }

        if (correct)
        {
            *value = temp;
            return true;
        }
        Logger::Error(Logger::Module::Core,
            Formatter::Format("Failed to parse value for an argument %s!", match));
        return false;
    }

    bool Parser::Value(const MC::StringView& line, const MC::StringView& match, u8* value) noexcept
    {
        if (value == nullptr)
        {
            Logger::Error(Logger::Module::Core,
                "Can't parse line without value output!");
            return false;
        }

        usize position = line.FindFirst(match);
        if (position == StringView::NoPosition) return false;

        usize valuePosition;
        auto temp = Convert<StringView>::ToU8(line.Substring(position + match.Size()), &valuePosition);
        bool correct = true;
        bool seenDigit = false;
        for (usize i = 0; i < valuePosition; ++i)
        {
            char8 ch = line[position + match.Size() + i];
            if (ch == ' ') continue;
            if (ch == '-' && !seenDigit) { seenDigit = true; continue; }
            if (ch == '-') { correct = false; break; }
            seenDigit = true;
        }

        if (correct)
        {
            *value = temp;
            return true;
        }
        Logger::Error(Logger::Module::Core,
            Formatter::Format("Failed to parse value for an argument %s!", match));
        return false;
    }

    bool Parser::Value(const MC::StringView& line, const MC::StringView& match, i64* value) noexcept
    {
        if (value == nullptr)
        {
            Logger::Error(Logger::Module::Core,
                "Can't parse line without value output!");
            return false;
        }

        usize position = line.FindFirst(match);
        if (position == StringView::NoPosition) return false;

        usize valuePosition;
        auto temp = Convert<StringView>::ToI64(line.Substring(position + match.Size()), &valuePosition);
        bool correct = true;
        bool seenDigit = false;
        for (usize i = 0; i < valuePosition; ++i)
        {
            char8 ch = line[position + match.Size() + i];
            if (ch == ' ') continue;
            if (ch == '-' && !seenDigit) { seenDigit = true; continue; }
            if (ch == '-') { correct = false; break; }
            seenDigit = true;
        }

        if (correct)
        {
            *value = temp;
            return true;
        }
        Logger::Error(Logger::Module::Core,
            Formatter::Format("Failed to parse value for an argument %s!", match));
        return false;
    }

    bool Parser::Value(const MC::StringView& line, const MC::StringView& match, i32* value) noexcept
    {
        if (value == nullptr)
        {
            Logger::Error(Logger::Module::Core,
                "Can't parse line without value output!");
            return false;
        }

        usize position = line.FindFirst(match);
        if (position == StringView::NoPosition) return false;

        usize valuePosition;
        auto temp = Convert<StringView>::ToI32(line.Substring(position + match.Size()), &valuePosition);
        bool correct = true;
        bool seenDigit = false;
        for (usize i = 0; i < valuePosition; ++i)
        {
            char8 ch = line[position + match.Size() + i];
            if (ch == ' ') continue;
            if (ch == '-' && !seenDigit) { seenDigit = true; continue; }
            if (ch == '-') { correct = false; break; }
            seenDigit = true;
        }

        if (correct)
        {
            *value = temp;
            return true;
        }
        Logger::Error(Logger::Module::Core,
            Formatter::Format("Failed to parse value for an argument %s!", match));
        return false;
    }

    bool Parser::Value(const MC::StringView& line, const MC::StringView& match, i16* value) noexcept
    {
        if (value == nullptr)
        {
            Logger::Error(Logger::Module::Core,
                "Can't parse line without value output!");
            return false;
        }

        usize position = line.FindFirst(match);
        if (position == StringView::NoPosition) return false;

        usize valuePosition;
        auto temp = Convert<StringView>::ToI16(line.Substring(position + match.Size()), &valuePosition);
        bool correct = true;
        bool seenDigit = false;
        for (usize i = 0; i < valuePosition; ++i)
        {
            char8 ch = line[position + match.Size() + i];
            if (ch == ' ') continue;
            if (ch == '-' && !seenDigit) { seenDigit = true; continue; }
            if (ch == '-') { correct = false; break; }
            seenDigit = true;
        }

        if (correct)
        {
            *value = temp;
            return true;
        }
        Logger::Error(Logger::Module::Core,
            Formatter::Format("Failed to parse value for an argument %s!", match));
        return false;
    }

    bool Parser::Value(const MC::StringView& line, const MC::StringView& match, i8* value) noexcept
    {
        if (value == nullptr)
        {
            Logger::Error(Logger::Module::Core,
                "Can't parse line without value output!");
            return false;
        }

        usize position = line.FindFirst(match);
        if (position == StringView::NoPosition) return false;

        usize valuePosition;
        auto temp = Convert<StringView>::ToI8(line.Substring(position + match.Size()), &valuePosition);
        bool correct = true;
        bool seenDigit = false;
        for (usize i = 0; i < valuePosition; ++i)
        {
            char8 ch = line[position + match.Size() + i];
            if (ch == ' ') continue;
            if (ch == '-' && !seenDigit) { seenDigit = true; continue; }
            if (ch == '-') { correct = false; break; }
            seenDigit = true;
        }

        if (correct)
        {
            *value = temp;
            return true;
        }
        Logger::Error(Logger::Module::Core,
            Formatter::Format("Failed to parse value for an argument %s!", match));
        return false;
    }

    bool Parser::Value(const MC::StringView& line, const MC::StringView& match, f64* value) noexcept
    {
        if (value == nullptr)
        {
            Logger::Error(Logger::Module::Core,
                "Can't parse line without value output!");
            return false;
        }

        usize position = line.FindFirst(match);
        if (position == StringView::NoPosition) return false;

        usize valuePosition;
        auto temp = Convert<StringView>::ToF64(line.Substring(position + match.Size()), &valuePosition);
        bool correct = true;
        bool seenDigit = false;
        for (usize i = 0; i < valuePosition; ++i)
        {
            char8 ch = line[position + match.Size() + i];
            if (ch == ' ') continue;
            if (ch == '-' && !seenDigit) { seenDigit = true; continue; }
            if (ch == '-') { correct = false; break; }
            seenDigit = true;
        }

        if (correct)
        {
            *value = temp;
            return true;
        }
        Logger::Error(Logger::Module::Core,
            Formatter::Format("Failed to parse value for an argument %s!", match));
        return false;
    }

    bool Parser::Value(const MC::StringView& line, const MC::StringView& match, f32* value) noexcept
    {
        if (value == nullptr)
        {
            Logger::Error(Logger::Module::Core,
                "Can't parse line without value output!");
            return false;
        }

        usize position = line.FindFirst(match);
        if (position == StringView::NoPosition) return false;

        usize valuePosition;
        auto temp = Convert<StringView>::ToF32(line.Substring(position + match.Size()), &valuePosition);
        bool correct = true;
        bool seenDigit = false;
        for (usize i = 0; i < valuePosition; ++i)
        {
            char8 ch = line[position + match.Size() + i];
            if (ch == ' ') continue;
            if (ch == '-' && !seenDigit) { seenDigit = true; continue; }
            if (ch == '-') { correct = false; break; }
            seenDigit = true;
        }

        if (correct)
        {
            *value = temp;
            return true;
        }
        Logger::Error(Logger::Module::Core,
            Formatter::Format("Failed to parse value for an argument %s!", match));
        return false;
    }

    bool Parser::Value(const MC::StringView& line, const MC::StringView& match, const char8** value, usize* size) noexcept
    {
        if (value == nullptr || size == nullptr)
        {
            Logger::Error(Logger::Module::Core,
                "Can't parse line without value output!");
            return false;
        }

        *value = nullptr;
        *size = 0;

        usize position = line.FindFirst(match);
        if (position == StringView::NoPosition) return false;

        const char8* run = line.String() + position + match.Size();

        while (*run == ' ')
        {
            if (*run == '\0') return false;
            ++run;
        }

        if (*run == '-')
        {
            Logger::Error(Logger::Module::Core,
                "Error: Unexpected '-' sign at the beginning of the value.");
            return false;
        }

        char8 target = ' ';
        if (*run == '\"' || *run == '\'')
        {
            target = *run;
            ++run;
        }

        const char8* value_start = run;
        if (target == ' ')
        {
            while (*run != '\0')
            {
                if (*run == ' ' || *run == '-') break;
                ++run;
            }

            if (run == value_start) return false;
        }
        else
        {
            while (*run != '\0' && *run != target) ++run;

            if (*run != target)
                return false;
        }

        *value = value_start;
        *size = static_cast<usize>(run - value_start);

        return true;
    }

    bool Parser::Line(const MC::StringView& line, const char8** result, usize* size) noexcept
    {
        if (result == nullptr || size == nullptr)
        {
            Logger::Error(Logger::Module::Core,
                "Can't parse line without value output!");
            return false;
        }

        *result = line.String();

        const char8* run = *result;
        while (*run != '\n')
        {
            ++run;
            if (*run == '\0') break;
        }

        *size = static_cast<usize>(run - *result);
        return true;
    }

    bool Parser::Line(const MC::StringView& line, char8* result, usize resultMaxSize) noexcept
    {
        if (result == nullptr || resultMaxSize == 0)
        {
            Logger::Error(Logger::Module::Core,
                "Can't parse line without value output!");
            return false;
        }

        const char8* run = line.String();
        while (*run != '\n')
        {
            ++run;
            if (*run == '\0') break;
        }

        usize size = static_cast<usize>(run - line.String());
        size = Min(size, resultMaxSize - 1);

        memcpy(result, line.String(), size);
        result[size] = '\0';
        return true;
    }

    bool Parser::Line(const MC::StringView& line, MC::String* result) noexcept
    {
        if (result == nullptr)
        {
            Logger::Error(Logger::Module::Core,
                "Can't parse line without value output!");
            return false;
        }

        const char8* run = line.String();
        while (*run != '\n')
        {
            ++run;
            if (*run == '\0') break;
        }

        usize size = static_cast<usize>(run - line.String());
        result->Resize(size);

        memcpy(result->String(), line.String(), size);

        return true;
    }
}
