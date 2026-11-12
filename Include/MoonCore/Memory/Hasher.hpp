#pragma once
#include "MoonCore/Core.hpp"

namespace MC::Concept
{
	template <class T>
	concept HasherType = requires
	{
		typename T::ValueType;
	}
	&& requires(T hasher, typename T::ValueType* data, usize size, usize tableSize)
	{
		noexcept(hasher.Hash(data, size, tableSize));
		{ hasher.Hash(data, size, tableSize) } -> ConvertibleTo<usize>;
	};
}

namespace MC::Memory
{
    namespace Helper
    {
        constexpr u64 FNV_PRIMC = 1099511628211ULL;
        constexpr u64 FNV_OFFSET_BASIS = 14695981039346656037ULL;
    }

	usize Hash64(const void* input, usize length, u64 seed) noexcept;

	template <typename T>
	struct Hasher
	{
		static_assert(IsTriviallyCopyable<T>, "T of Hasher must be trivially copyable");
	
	public:
		using ValueType = T;

	public:
		template<bool TableSizeIsUsed = true>
		MC_NODISCARD static usize Hash(const ValueType* data, usize size, usize tableSize, u64 seed = 0) noexcept
		{
			if constexpr (TableSizeIsUsed)
			{
				if constexpr (sizeof(T) <= 8)
				{
					if (size == 1)
					{
						usize buf = 0;
						*reinterpret_cast<ValueType*>(&buf) = *data;
					    return (buf + seed) % tableSize;
					}
				}
			    return Hash64(data, sizeof(ValueType) * size, seed) % tableSize;
			}
			else
			{
				if constexpr (sizeof(T) <= 8)
				{
					if (size == 1)
					{
						usize buf = 0;
						*reinterpret_cast<ValueType*>(&buf) = *data;
						return (buf + seed);
					}
				}
				return Hash64(data, sizeof(ValueType) * size, seed);
			}
		}
	};

	template <Concept::Character CharacterType>
	struct Hasher<const CharacterType*>
	{
	public:
		using ValueType = const CharacterType*;

	public:
		template<bool TableSizeIsUsed = true>
		MC_NODISCARD static usize Hash(ValueType data, usize size, usize tableSize, u64 seed = 0) noexcept
		{
			if constexpr (TableSizeIsUsed)
			{
				if constexpr (sizeof(CharacterType) <= 8)
				{
					if (size == 1)
					{
						usize buf = 0;
						*reinterpret_cast<usize*>(&buf) = *data;
						return (buf + seed) % tableSize;
					}
				}
				return Hash64(data, sizeof(CharacterType) * size, seed) % tableSize;
			}
			else
			{
				if constexpr (sizeof(CharacterType) <= 8)
				{
					if (size == 1)
					{
						usize buf = 0;
						*reinterpret_cast<usize*>(&buf) = *data;
						return (buf + seed);
					}
				}
				return Hash64(data, sizeof(CharacterType) * size, seed);
			}
		}
	};

	template<>
	struct Hasher<void>
	{
	public: 
		using ValueType = void;

	public:
		template<bool TableSizeIsUsed = true>
		MC_NODISCARD static usize Hash(const ValueType* data, usize size, usize tableSize, u64 seed = 0) noexcept
		{
			if constexpr (TableSizeIsUsed)
			{
				if (size <= 8)
				{
					usize buf = 0;
					memcpy_s(&buf, sizeof(usize), data, size);
			        return (buf + seed) % tableSize;
				}
				return Hash64(data, size, seed) % tableSize;
			}
			else
			{
				if (size <= 8)
				{
					usize buf = 0;
					memcpy_s(&buf, sizeof(usize), data, size);
					return (buf + seed);
				}
				return Hash64(data, size * size, seed);
			}
		}
	};

    template <typename ValueType> struct ConstexprHasher;

	template<>
	struct ConstexprHasher<const char7*>
	{
		constexpr static usize Hash(const char7* data, usize length, usize tableSize) noexcept
		{
			usize hash = Helper::FNV_OFFSET_BASIS;
			for (usize i = 0; i < length; ++i)
			{
				hash ^= static_cast<usize>(data[i]);
				hash *= Helper::FNV_PRIMC;
			}
			return tableSize == 0 ? hash : hash % tableSize;
		}
	};

	template<>
	struct ConstexprHasher<const char8*>
	{
		MC_NODISCARD constexpr static usize Hash(const char8* data, usize length, usize tableSize) noexcept
		{
			usize hash = Helper::FNV_OFFSET_BASIS;
			for (usize i = 0; i < length; ++i)
			{
				hash ^= static_cast<usize>(data[i]);
				hash *= Helper::FNV_PRIMC;
			}
			return tableSize == 0 ? hash : hash % tableSize;
		}
	};

	template<>
	struct ConstexprHasher<const wchar*>
	{
		MC_NODISCARD constexpr static usize Hash(const wchar* const data, usize length, usize tableSize) noexcept
		{
			usize hash = Helper::FNV_OFFSET_BASIS;
			for (usize i = 0; i < length; ++i)
			{
				hash ^= static_cast<usize>(data[i]);
				hash *= Helper::FNV_PRIMC;
			}
			return tableSize == 0 ? hash : hash % tableSize;
		}
	};
}