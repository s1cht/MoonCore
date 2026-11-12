#pragma once
#include "String/String.hpp"
#include "String/StringView.hpp"
#include "String/StringUtils.hpp"
#include "MoonCore/Memory/Hasher.hpp"

namespace MC
{
	constexpr const char7* AsChar7(const String& str) noexcept
	{
		if (IsConstantEvaluated())
			return "";
	    return AS_CHAR7(str.String());
	}
	constexpr const char7* AsChar7(const StringView& str) noexcept
	{
		if (IsConstantEvaluated())
			return "";
	    return AS_CHAR7(str.String());
	}

	constexpr const char8* AsChar8(const String& str) noexcept
	{
		if (IsConstantEvaluated())
			return TEXT("");
		return str.String();
	}
	constexpr const char8* AsChar8(const StringView& str) noexcept { return str.String(); }

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
		template<Concept::Character CharacterType>
		struct Hasher<StringViewImpl<CharacterType>>
		{
			using ValueType = StringViewImpl<CharacterType>;

		public:
			MC_NODISCARD static usize Hash(const ValueType* const str, usize, usize tableSize = 0, u64 seed = 0) noexcept
			{
				return tableSize == 0 ? Hash64(str->String(), str->Size() * sizeof(ValueType::ValueType), seed) :
					Hash64(str->String(), str->Size() * sizeof(ValueType::ValueType), seed) % tableSize;
			}
		};

		template<Concept::Character CharacterType, Concept::HeapAllocatorType AllocatorType>
		struct Hasher<StringImpl<CharacterType, AllocatorType>>
		{
			using ValueType = StringImpl<CharacterType, AllocatorType>;

		public:
			MC_NODISCARD static usize Hash(const ValueType* const str, usize, usize tableSize = 0, u64 seed = 0) noexcept
			{
				return tableSize == 0 ? Hash64(str->String(), str->Size() * sizeof(ValueType::ValueType), seed) :
					Hash64(str->String(), str->Size() * sizeof(ValueType::ValueType), seed) % tableSize;
			}
		};

		template<Concept::Character CharacterType>
		struct Memory::ConstexprHasher<StringViewImpl<CharacterType>>
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