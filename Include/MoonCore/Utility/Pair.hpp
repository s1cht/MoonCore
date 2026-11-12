#pragma once
#include "MoonCore/Core.hpp"

namespace MC::Utility
{
	template <typename KeyT, typename ValueT>
	struct Pair
	{
	public:
		using KeyType = KeyT;
		using ValueType = ValueT;

	public:
		constexpr Pair() noexcept = default;
		constexpr Pair(const KeyType& key, const ValueType& value) noexcept : Key(key), Value(value) {}
		constexpr Pair(KeyType&& key, ValueType&& value) noexcept : Key(Move(key)), Value(Move(value)) {}
		constexpr Pair(const Pair& other) noexcept : Key(other.Key), Value(other.Value) {}
		constexpr Pair(Pair&& other) noexcept : Key(Move(other.Key)), Value(Move(other.Value)) {}

	public:
		constexpr Pair& operator=(const Pair& other) noexcept
		{
		    if (this != &other)
		    {
				Key = other.Key;
				Value = other.Value;
		    }
			return *this;
		}
		constexpr Pair& operator=(Pair&& other) noexcept
		{
			if (this != &other)
			{
				Key = Move(other.Key);  
			    Value = Move(other.Value);
			}
			return *this;
		}

	public:
		MC_NODISCARD constexpr bool operator==(const Pair& other) noexcept
		{
		    return Key == other.Key && Value == other.Value;
		}

	public:
		KeyType Key;
		ValueType Value;
	};
}