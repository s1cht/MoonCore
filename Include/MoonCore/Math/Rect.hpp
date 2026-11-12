#pragma once
#include "MathCore.hpp"

namespace MC::Math
{
	template<Concept::Number T>
	struct Resolution2D
	{
	public:
		bool operator==(const Resolution2D& other) const noexcept
		{
		    if constexpr (IsFloatingPoint<T>)
				return x - other.x <= NumericLimit<T>::Epsilon && y - other.y <= NumericLimit<T>::Epsilon;
			else
				return x == other.x && y == other.y;
		}

		template<Concept::Number U>
		operator Resolution2D<U>() const noexcept
		{
			return { static_cast<U>(x), static_cast<U>(y) };
		}

		constexpr Resolution2D operator+(const Resolution2D& other) const noexcept { return { x + other.x, y + other.y }; }
		constexpr Resolution2D& operator+=(const Resolution2D& other) noexcept { x += other.x; y += other.y; return *this; }
		constexpr Resolution2D operator-(const Resolution2D& other) const noexcept { return { x - other.x, y - other.y }; }
		constexpr Resolution2D& operator-=(const Resolution2D& other) noexcept { x -= other.x; y -= other.y; return *this; }

	public:
		T x;
		T y;
	};

	template<Concept::Number T>
	struct Resolution3D
	{
	public:
		constexpr Resolution3D operator+(const Resolution3D& other) const noexcept { return { x + other.x, y + other.y, z + other.z }; }
		constexpr Resolution3D& operator+=(const Resolution3D& other) noexcept { x += other.x; y += other.y; z += other.z; return *this; }
		constexpr Resolution3D operator-(const Resolution3D& other) const noexcept { return { x - other.x, y - other.y, z - other.z }; }
		constexpr Resolution3D& operator-=(const Resolution3D& other) noexcept { x -= other.x; y -= other.y; z -= other.z; return *this; }
	
	public:
		T x;
		T y;
		T z;
	};

	struct Rect2D
	{
		Resolution2D<u32> Offset;
		Resolution2D<u32> Extent;
	};
}

MC_FMT_FORMATTER_TEMPLATE(<MC::Concept::Number T>, MC::Math::Resolution2D<T>, "%f, %f", MC_FMT_FORMATTER_MEMBER(x), MC_FMT_FORMATTER_MEMBER(y));
MC_FMT_FORMATTER_TEMPLATE(<MC::Concept::Number T>, MC::Math::Resolution3D<T>, "%f, %f, %f", MC_FMT_FORMATTER_MEMBER(x), MC_FMT_FORMATTER_MEMBER(y), MC_FMT_FORMATTER_MEMBER(z));
MC_FMT_FORMATTER(MC::Math::Rect2D, "Offset: %f; Extent: %f", MC_FMT_FORMATTER_MEMBER(Offset), MC_FMT_FORMATTER_MEMBER(Extent));