#pragma once
#include "MoonCore/Utility/Details/FmtTraits.inl"
#include "MoonCore/Containers/String.hpp"
#include "MoonCore/Utility/Iterator.hpp"

namespace MC
{
    namespace Details
    {
        using MemoryBuffer = External::fmt::basic_memory_buffer<char8>;
        using Appender = External::fmt::basic_appender<char8>;
        using FormatStringView = External::fmt::basic_string_view<char8>;
        template <typename... T>
        inline auto Runtime(FormatStringView s) noexcept -> External::fmt::runtime_format_string<char8> {
            return { {s.data(), s.size()} };
        }

        class StringWrapper
        {
        public:
            using value_type = String::ValueType;
            using ValueType  = String::ValueType;

        public:
            StringWrapper(String& str) noexcept : m_String(&str) {}

        public:
            void push_back(const value_type& value) const noexcept { m_String->PushBack(value); }
            void PushBack(const value_type& value) const noexcept { m_String->PushBack(value); }
            void push_back(value_type&& value) const noexcept { m_String->PushBack(Move(value)); }
            void PushBack(value_type&& value) const noexcept { m_String->PushBack(Move(value)); }

        private:
            String* m_String;
        };
    }

    class Formatter
    {
    public:
        template<typename... ArgTypes>
        static MC::String Format(const MC::StringView& fmt, ArgTypes&&... args) noexcept;

        template<typename... ArgTypes>
        static void FormatTo(MC::String& outTarget, const MC::StringView& fmt, ArgTypes&&... args) noexcept;
    };

    template <typename... ArgTypes>
    MC::String Formatter::Format(const MC::StringView& fmt, ArgTypes&&  ... args) noexcept
    {
        auto buffer = Details::MemoryBuffer();
        External::fmt::vformat_to(
            Details::Appender(buffer),
            Details::FormatStringView(fmt.String(), fmt.Size()),
            External::fmt::make_format_args
            <External::fmt::generic_context<Details::Appender, char8>>(args...)
        );
        return { buffer.data(), buffer.size() };
    }

    template <typename... ArgTypes>
    void Formatter::FormatTo(MC::String& outTarget, const MC::StringView& fmt, ArgTypes&&... args) noexcept
    {
        auto wrapper = Details::StringWrapper(outTarget);
        External::fmt::vformat_to(
            MC::Iterator::BackInsertIterator(wrapper),
            Details::FormatStringView(fmt.String(), fmt.Size()),
            External::fmt::make_format_args
            <External::fmt::generic_context<Details::Appender, char8>>(args...)
        );
    }
}

MC_FMT_FORMATTER_UTF7(MC::StringView, "{}", MC::AsChar7(MC_FMT_FORMATTER_VALUE()));
MC_FMT_FORMATTER_UTF8(MC::StringView, "{}", MC_FMT_FORMATTER_METHOD(String));

MC_FMT_FORMATTER_UTF7(MC::String, "{}", MC::AsChar7(MC_FMT_FORMATTER_VALUE()));
MC_FMT_FORMATTER_UTF8(MC::String, "{}", MC_FMT_FORMATTER_METHOD(String));