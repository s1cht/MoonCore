#pragma once
#include <limits.h>
#include <float.h>
#include "Macros.hpp"

// Compiler defines
#if defined(COMPILER_MSVC)
#	include "Platform/MSVCDefines.inl"
#endif

// Platform types
#if defined(PLATFORM_WINDOWS)
	#	include "MoonCore/Platform/WindowsTypes.inl"
#elif defined(PLATFORM_UNIX)
	#	include "MoonCore/Platform/Unix/UnixTypes.hpp"
#else
#	include "MoonCore/Platform/Base/BaseTypes.hpp"
    typedef BaseTypes PlatformTypes;
#endif 

/*							Characters								*/

// Ansi character
typedef PlatformTypes::char7	char7;

// Wide character
typedef PlatformTypes::wchar	wchar;

// UTF-8 char
typedef PlatformTypes::char8	char8;

// UTF-16 char
typedef PlatformTypes::char16	char16;

// UTF-32 char
typedef PlatformTypes::char32	char32;

/*							Integers								*/

// 8-bit integer
typedef PlatformTypes::i8		i8;

// 16-bit integer
typedef PlatformTypes::i16		i16;

// 32-bit integer
typedef PlatformTypes::i32		i32;

// 64-bit integer
typedef PlatformTypes::i64		i64;


// 8-bit unsigned integer
typedef PlatformTypes::u8		u8;

// 16-bit unsigned integer
typedef PlatformTypes::u16		u16;

// 32-bit unsigned integer
typedef PlatformTypes::u32		u32;

// 64-bit unsigned integer
typedef PlatformTypes::u64		u64;

// Signed IntPtr
typedef PlatformTypes::iptr		iptr;

// Unsigned IntPtr
typedef PlatformTypes::uptr		uptr;

// Signed size_t
typedef PlatformTypes::isize	isize;

// Unsigned size_t
typedef PlatformTypes::usize	usize;

/*					Numbers with floating point						*/

// 32-bit float
typedef PlatformTypes::f32		f32;

// 64-bit float
typedef PlatformTypes::f64		f64;

typedef PlatformTypes::lf64		lf64;

/*		String literals reinterpretation - standardisation macros	*/

#define TEXT_JOIN(x) u8 ## x
#define TEXT_HELPER(x) TEXT_JOIN(x)
#define TEXT(x) TEXT_HELPER(x)

#define TEXTW_JOIN(x) L ## x
#define TEXTW_HELPER(x) TEXTW_JOIN(x)
#define TEXTW(x) TEXTW_HELPER(x)

#define AS_CHAR7(text) reinterpret_cast<const char7*>(text)
#define AS_UTF8(text) reinterpret_cast<const char8*>(text)

namespace MC
{
	using MaxAlignT = f64;

	template<typename T> struct NumericLimit
	{
		inline static constexpr T Min          = T();
		inline static constexpr T Max          = T();
		inline static constexpr T Lowest       = T();
		inline static constexpr T Epsilon      = T();
		inline static constexpr T RoundError   = T();
		inline static constexpr T DenormMin    = T();
		inline static constexpr T Infinity     = T();
        inline static constexpr T QuietNaN     = T();
        inline static constexpr T SignalingNaN = T();
	};

	template<> struct NumericLimit<bool>
	{
		inline static constexpr bool Min          = false;
		inline static constexpr bool Max          = true;
		inline static constexpr bool Lowest		  = Min;
		inline static constexpr bool Epsilon	  = false;
		inline static constexpr bool RoundError   = false;
		inline static constexpr bool DenormMin	  = false;
		inline static constexpr bool Infinity	  = false;
		inline static constexpr bool QuietNaN	  = false;
		inline static constexpr bool SignalingNaN = false;
	};

#ifdef PLATFORM_WINDOWS
	template<> struct NumericLimit<wchar>
	{
		inline static constexpr wchar Min = 0x0000;
		inline static constexpr wchar Max = 0xFFFF;
		inline static constexpr wchar Lowest = Min;
		inline static constexpr wchar Epsilon = 0;
		inline static constexpr wchar RoundError = 0;
		inline static constexpr wchar DenormMin = 0;
		inline static constexpr wchar Infinity = 0;	
		inline static constexpr wchar QuietNaN = 0;
		inline static constexpr wchar SignalingNaN = 0;
	};
#else
	template<> struct NumericLimit<wchar>
	{
		inline static constexpr wchar Min = 0x00;
		inline static constexpr wchar Max = 0xFF;
		inline static constexpr wchar Lowest = Min;
		inline static constexpr wchar Epsilon = 0;
		inline static constexpr wchar RoundError = 0;
		inline static constexpr wchar DenormMin = 0;
		inline static constexpr wchar Infinity = 0;
		inline static constexpr wchar QuietNaN = 0;
		inline static constexpr wchar SignalingNaN = 0;
	};
#endif

	template<> struct NumericLimit<char7>
	{
		inline static constexpr char7 Min = CHAR_MIN;
		inline static constexpr char7 Max = CHAR_MAX;
		inline static constexpr char7 Lowest = Min;
		inline static constexpr char7 Epsilon = 0;
		inline static constexpr char7 RoundError = 0;
		inline static constexpr char7 DenormMin = 0;
		inline static constexpr char7 Infinity = 0;
		inline static constexpr char7 QuietNaN = 0;
		inline static constexpr char7 SignalingNaN = 0;
	};

	template<> struct NumericLimit<char8>
	{
		inline static constexpr char8 Min = 0x00;
		inline static constexpr char8 Max = 0xFF;
		inline static constexpr char8 Lowest = Min;
		inline static constexpr char8 Epsilon = 0;
		inline static constexpr char8 RoundError = 0;
		inline static constexpr char8 DenormMin = 0;
		inline static constexpr char8 Infinity = 0;
		inline static constexpr char8 QuietNaN = 0;
		inline static constexpr char8 SignalingNaN = 0;
	};

	template<> struct NumericLimit<char16>
	{
		inline static constexpr char16 Min = 0x00;
		inline static constexpr char16 Max = 0xFFFF;
		inline static constexpr char16 Lowest = Min;
		inline static constexpr char16 Epsilon = 0;
		inline static constexpr char16 RoundError = 0;
		inline static constexpr char16 DenormMin = 0;
		inline static constexpr char16 Infinity = 0;
		inline static constexpr char16 QuietNaN = 0;
		inline static constexpr char16 SignalingNaN = 0;
	};

	template<> struct NumericLimit<char32>
	{
		inline static constexpr char32 Min = 0x00;
		inline static constexpr char32 Max = 0xFFFFFFFF;
		inline static constexpr char32 Lowest = Min;
		inline static constexpr char32 Epsilon = 0;
		inline static constexpr char32 RoundError = 0;
		inline static constexpr char32 DenormMin = 0;
		inline static constexpr char32 Infinity = 0;
		inline static constexpr char32 QuietNaN = 0;
		inline static constexpr char32 SignalingNaN = 0;
	};

	template<> struct NumericLimit<i8>
	{
		inline static constexpr i8 Min = CHAR_MIN;
		inline static constexpr i8 Max = CHAR_MAX;
		inline static constexpr i8 Lowest = Min;
		inline static constexpr i8 Epsilon = 0;
		inline static constexpr i8 RoundError = 0;
		inline static constexpr i8 DenormMin = 0;
		inline static constexpr i8 Infinity = 0;
		inline static constexpr i8 QuietNaN = 0;
		inline static constexpr i8 SignalingNaN = 0;
	};

	template<> struct NumericLimit<i16>
	{
		inline static constexpr i16 Min = SHRT_MIN;
		inline static constexpr i16 Max = SHRT_MAX;
		inline static constexpr i16 Lowest = Min;
		inline static constexpr i16 Epsilon = 0;
		inline static constexpr i16 RoundError = 0;
		inline static constexpr i16 DenormMin = 0;
		inline static constexpr i16 Infinity = 0;
		inline static constexpr i16 QuietNaN = 0;
		inline static constexpr i16 SignalingNaN = 0;
	};

	template<> struct NumericLimit<i32>
	{
		inline static constexpr i32 Min = INT_MIN;
		inline static constexpr i32 Max = INT_MAX;
		inline static constexpr i32 Lowest = Min;
		inline static constexpr i32 Epsilon = 0;
		inline static constexpr i32 RoundError = 0;
		inline static constexpr i32 DenormMin = 0;
		inline static constexpr i32 Infinity = 0;
		inline static constexpr i32 QuietNaN = 0;
		inline static constexpr i32 SignalingNaN = 0;
	};

	template<> struct NumericLimit<i64>
	{
		inline static constexpr i64 Min = LLONG_MIN;
		inline static constexpr i64 Max = LLONG_MAX;
		inline static constexpr i64 Lowest = Min;
		inline static constexpr i64 Epsilon = 0;
		inline static constexpr i64 RoundError = 0;
		inline static constexpr i64 DenormMin = 0;
		inline static constexpr i64 Infinity = 0;
		inline static constexpr i64 QuietNaN = 0;
		inline static constexpr i64 SignalingNaN = 0;
	};

	template<> struct NumericLimit<u8>
	{
		inline static constexpr u8 Min = 0x00;
		inline static constexpr u8 Max = 0xFF;
		inline static constexpr u8 Lowest = Min;
		inline static constexpr u8 Epsilon = 0;
		inline static constexpr u8 RoundError = 0;
		inline static constexpr u8 DenormMin = 0;
		inline static constexpr u8 Infinity = 0;
		inline static constexpr u8 QuietNaN = 0;
		inline static constexpr u8 SignalingNaN = 0;
	};

	template<> struct NumericLimit<u16>
	{
		inline static constexpr u16 Min = 0x00;
		inline static constexpr u16 Max = 0xFFFF;
		inline static constexpr u16 Lowest = Min;
		inline static constexpr u16 Epsilon = 0;
		inline static constexpr u16 RoundError = 0;
		inline static constexpr u16 DenormMin = 0;
		inline static constexpr u16 Infinity = 0;
		inline static constexpr u16 QuietNaN = 0;
		inline static constexpr u16 SignalingNaN = 0;
	};

	template<> struct NumericLimit<u32>
	{
		inline static constexpr u32 Min = 0x00;
		inline static constexpr u32 Max = 0xFFFFFFFF;
		inline static constexpr u32 Lowest = Min;
		inline static constexpr u32 Epsilon = 0;
		inline static constexpr u32 RoundError = 0;
		inline static constexpr u32 DenormMin = 0;
		inline static constexpr u32 Infinity = 0;
		inline static constexpr u32 QuietNaN = 0;
		inline static constexpr u32 SignalingNaN = 0;
	};

	template<> struct NumericLimit<u64>
	{
		inline static constexpr u64 Min = 0x00;
		inline static constexpr u64 Max = 0xFFFFFFFFFFFFFFFF;
		inline static constexpr u64 Lowest = Min;
		inline static constexpr u64 Epsilon = 0;
		inline static constexpr u64 RoundError = 0;
		inline static constexpr u64 DenormMin = 0;
		inline static constexpr u64 Infinity = 0;
		inline static constexpr u64 QuietNaN = 0;
		inline static constexpr u64 SignalingNaN = 0;
	};

	template<> struct NumericLimit<f32>
	{
		inline static constexpr f32 Min = FLT_MIN;
		inline static constexpr f32 Max = FLT_MAX;
		inline static constexpr f32 Lowest = -Max;
		inline static constexpr f32 Epsilon = FLT_EPSILON;
		inline static constexpr f32 RoundError = 0.5f;
		inline static constexpr f32 DenormMin = FLT_TRUE_MIN;
		inline static constexpr f32 Infinity = __builtin_huge_valf();
		inline static constexpr f32 QuietNaN = __builtin_nanf("0");
		inline static constexpr f32 SignalingNaN = __builtin_nansf("1");
	};

	template<> struct NumericLimit<f64>
	{
		inline static constexpr f64 Min = DBL_MIN;
		inline static constexpr f64 Max = DBL_MAX;
		inline static constexpr f64 Lowest = -Max;
		inline static constexpr f64 Epsilon = DBL_EPSILON;
		inline static constexpr f64 RoundError = 0.5;
		inline static constexpr f64 DenormMin = DBL_TRUE_MIN;
		inline static constexpr f64 Infinity = __builtin_huge_val();
		inline static constexpr f64 QuietNaN = __builtin_nan("0");
		inline static constexpr f64 SignalingNaN = __builtin_nans("1");
	};

	template<> struct NumericLimit<lf64>
	{
		inline static constexpr lf64 Min = LDBL_MIN;
		inline static constexpr lf64 Max = LDBL_MAX;
		inline static constexpr lf64 Lowest = -Max;
		inline static constexpr lf64 Epsilon = LDBL_EPSILON;
		inline static constexpr lf64 RoundError = 0.5L;
		inline static constexpr lf64 DenormMin = LDBL_TRUE_MIN;
		inline static constexpr lf64 Infinity = __builtin_huge_val();
		inline static constexpr lf64 QuietNaN = __builtin_nan("0");
		inline static constexpr lf64 SignalingNaN = __builtin_nans("1");
	};


    #define SUPPORT_BITWISE_FOR_ENUM_CLASS(Enum)											\
	constexpr Enum operator|(Enum left, Enum right) noexcept								\
	{																						\
		return static_cast<Enum>(Core::ToUnderlying(left) | Core::ToUnderlying(right));		\
	}																						\
	constexpr Enum& operator|=(Enum& left, Enum right) noexcept								\
	{																						\
		left = left | right;																\
		return left;																		\
	}																						\
	constexpr Enum operator&(Enum left, Enum right) noexcept								\
	{																						\
		return static_cast<Enum>(Core::ToUnderlying(left) & Core::ToUnderlying(right));		\
	}

	/*								String reinterpretation							*/
	constexpr const char7* AsChar7(const char7* str) noexcept { return str; }
	inline const char7* AsChar7(const char8* str) noexcept { return AS_CHAR7(str); }

	inline const char8* AsChar8(const char7* str) noexcept { return AS_UTF8(str); }
	inline const char8* AsChar8(const char8* str) noexcept { return str; }
}