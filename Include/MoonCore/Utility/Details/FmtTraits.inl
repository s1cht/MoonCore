#pragma once
#include "FormatIncludes.inl"

#define MC_FMT_FORMATTER_VALUE() object
#define MC_FMT_FORMATTER_MEMBER(member) object.member
#define MC_FMT_FORMATTER_METHOD(method) object.method()

#define MC_FMT_FORMATTER_STRUCT_TEMPLATE(chType, templateParameters, type, str, ...)          \
	template templateParameters														          \
    struct MC::External::fmt::formatter<type, chType>								          \
	{																				          \
	    constexpr auto parse(MC::External::fmt::parse_context<chType>& ctx) { return ctx.begin(); }	          \
	    template <typename FormatContext>											          \
	    auto format(const type& object, FormatContext& ctx) const noexcept \
		{																			          \
	        return format_to(ctx.out(), str, __VA_ARGS__);							          \
	    }																			          \
	};																					
#define MC_FMT_FORMATTER(type, str, ...)									   \
    MC_FMT_FORMATTER_STRUCT_TEMPLATE(char7, <>, type, str, __VA_ARGS__);	   \
    MC_FMT_FORMATTER_STRUCT_TEMPLATE(char8, <>, type, TEXT(str), __VA_ARGS__);		
#define MC_FMT_FORMATTER_TEMPLATE(templateParameters, type, str, ...)						   \
    MC_FMT_FORMATTER_STRUCT_TEMPLATE(char7, templateParameters, type, str, __VA_ARGS__);	   \
    MC_FMT_FORMATTER_STRUCT_TEMPLATE(char8, templateParameters, type, TEXT(str), __VA_ARGS__);	

#define MC_FMT_FORMATTER_UTF7(type, str, ...) MC_FMT_FORMATTER_STRUCT_TEMPLATE(char7, <>, type, str, __VA_ARGS__);
#define MC_FMT_FORMATTER_UTF8(type, str, ...) MC_FMT_FORMATTER_STRUCT_TEMPLATE(char8, <>, type, TEXT(str), __VA_ARGS__);