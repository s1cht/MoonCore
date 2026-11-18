#pragma once
#include "MoonCore/Containers/String/String.hpp"
#include "MoonCore/Containers/String/StringView.hpp"
#include "MoonCore/Containers/String/StringUtils.hpp"
#include "MoonCore/Memory/Hasher.hpp"

/**
 * @file String.hpp
 * @brief Umbrella header for string containers, string views, character conversions, and hashers.
 * @ingroup string_containers
 */

namespace MC
{
	/**
	 * @brief Converts a UTF-8 String pointer to an ANSI const char7* pointer.
	 * @param str Source string.
	 * @return Const pointer to char7 buffer.
	 */
	constexpr const char7* AsChar7(const String& str) noexcept
	{
		if (IsConstantEvaluated())
			return "";
	    return AS_CHAR7(str.String());
	}

	/**
	 * @brief Converts a UTF-8 StringView to an ANSI const char7* pointer.
	 * @param str Source string view.
	 * @return Const pointer to char7 buffer.
	 */
	constexpr const char7* AsChar7(const StringView& str) noexcept
	{
		if (IsConstantEvaluated())
			return "";
	    return AS_CHAR7(str.String());
	}

	/**
	 * @brief Accesses raw const char8* buffer of a String.
	 * @param str Source string.
	 * @return Pointer to UTF-8 character buffer.
	 */
	constexpr const char8* AsChar8(const String& str) noexcept
	{
		if (IsConstantEvaluated())
			return TEXT("");
		return str.String();
	}

	/**
	 * @brief Accesses raw const char8* buffer of a StringView.
	 * @param str Source string view.
	 * @return Pointer to UTF-8 character buffer.
	 */
	constexpr const char8* AsChar8(const StringView& str) noexcept { return str.String(); }

	/**
	 * @brief Converts a WideStringView (UTF-16) to an ASCII/UTF-8 String.
	 * @param str Source wide string view.
	 * @return Converted String.
	 */
	inline String WideStringViewToString(const WideStringView& str) noexcept
	{
		usize num;
		usize strSize = str.Size() + 1;
		char7* resultStr = new(std::nothrow) char7[strSize];

		errno_t err = wcstombs_s(&num, resultStr, strSize, str.String(), str.Size());
		if (err != 0)
		{
			delete[] resultStr;
			return "";
		}

		String result = String(resultStr, num - 1);
		delete[] resultStr;
		return result;
	}

	/**
	 * @brief Converts a StringView (UTF-8) to a WideString (UTF-16).
	 * @param str Source string view.
	 * @return Converted WideString.
	 */
	inline WideString StringViewToWideString(const StringView& str) noexcept
	{
		usize num;
		usize strSize = str.Size() + 1;
		wchar* resultStr = new(std::nothrow) wchar[strSize];
		errno_t err = mbstowcs_s(&num, resultStr, strSize, AsChar7(str), str.Size());

		if (err != 0)
		{
			delete[] resultStr;
			return L"";
		}

		WideString result = WideString(resultStr, num - 1);
		delete[] resultStr;
		return result;
	}
	namespace Memory
	{
		/** @brief xxHash64 specialization for `MC::StringViewImpl`. */
		template<Concept::Character CharacterType>
		struct Hasher<StringViewImpl<CharacterType>>
		{
			using ValueType = StringViewImpl<CharacterType>;

		public:
			MOON_NODISCARD static usize Hash(const ValueType* const str, usize, usize tableSize = 0, u64 seed = 0) noexcept
			{
				return tableSize == 0 ? Hash64(str->String(), str->Size() * sizeof(ValueType::ValueType), seed) :
					Hash64(str->String(), str->Size() * sizeof(ValueType::ValueType), seed) % tableSize;
			}
		};

		/** @brief xxHash64 specialization for `MC::StringImpl`. */
		template<Concept::Character CharacterType, Concept::HeapAllocatorType AllocatorType>
		struct Hasher<StringImpl<CharacterType, AllocatorType>>
		{
			using ValueType = StringImpl<CharacterType, AllocatorType>;

		public:
			MOON_NODISCARD static usize Hash(const ValueType* const str, usize, usize tableSize = 0, u64 seed = 0) noexcept
			{
				return tableSize == 0 ? Hash64(str->String(), str->Size() * sizeof(ValueType::ValueType), seed) :
					Hash64(str->String(), str->Size() * sizeof(ValueType::ValueType), seed) % tableSize;
			}
		};

		/** @brief Compile-time FNV-1a hasher for `MC::StringViewImpl`. */
		template<Concept::Character CharacterType>
		struct ConstexprHasher<StringViewImpl<CharacterType>>
		{
			constexpr static usize Hash(const StringViewImpl<CharacterType>& str, usize, usize tableSize) noexcept
			{
				usize hash = Helper::FNV_OFFSET_BASIS;
				for (auto& character : str)
				{
					hash ^= static_cast<usize>(character);
					hash *= Helper::FNV_PRIMC;
				}
				return tableSize == 0 ? hash : hash % tableSize;
			}
		};
	}
}