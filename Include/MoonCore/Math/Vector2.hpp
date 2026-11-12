#pragma once
#include "MoonCore/Math/MathCore.hpp"
#include "MoonCore/Math/Rect.hpp"

namespace MC::Math
{
	template<Concept::FloatingPoint T>
	struct Vector2
	{
	public:
		union
		{
			struct { T x, y; };
			struct { T X, Y; };
			T XY[2];
		};

	public:
		static const Vector2 ZeroVector;
		static const Vector2 OneVector;
        static const Vector2 RightVector;
        static const Vector2 LeftVector;
        static const Vector2 UpVector;
        static const Vector2 DownVector;
        static const Vector2 xAxis;
		static const Vector2 yAxis;

	public:
		explicit constexpr Vector2() noexcept
			: x(0), y(0)
		{}
		explicit constexpr Vector2(T scalar) noexcept
			: x(scalar), y(scalar)
		{}
		constexpr Vector2(T xValue, T yValue) noexcept
			: x(xValue), y(yValue)
		{}
		template<Concept::Number N>
		constexpr Vector2(Resolution2D<N> resolution) noexcept
			: x(static_cast<T>(resolution.x)), y(static_cast<T>(resolution.y))
		{}
		constexpr Vector2(T vector[2]) noexcept
			: x(vector[0]), y(vector[1])
		{}

		constexpr Vector2(const Vector2& otherVec) noexcept = default;
		constexpr Vector2(Vector2&& otherVec) noexcept = default;
		constexpr Vector2& operator=(const Vector2&) noexcept = default;
		constexpr Vector2& operator=(Vector2&&) noexcept = default;
		constexpr ~Vector2() noexcept = default;

	public:
	    bool operator==(const Vector2& other) const noexcept
		{
			return fabsf(x - other.x) < Epsilon<T> &&
				   fabsf(y - other.y) < Epsilon<T>;
		}

		constexpr Vector2 operator-() const noexcept
		{
			return { -x, -y };
		}

		constexpr Vector2 operator+(const Vector2& other) const noexcept
		{
			return { x + other.x, y + other.y };
		}
		constexpr Vector2 operator-(const Vector2& other) const noexcept
		{
			return { x - other.x, y - other.y };
		}
		constexpr Vector2 operator*(const Vector2& other) const noexcept
		{
			return { x * other.x, y * other.y };
		}
		Vector2 operator/(const Vector2& other) const noexcept
		{
			return {
				fabsf(other.x) > Epsilon<T> ? x / other.x : 0,
				fabsf(other.y) > Epsilon<T> ? y / other.y : 0
			};
		}

		template<Concept::Number U>
		constexpr Vector2 operator*(U scalar) const noexcept
		{
			return { x * static_cast<T>(scalar), y * static_cast<T>(scalar) };
		}
		template<Concept::Number U>
		constexpr Vector2 operator/(U scalar) const noexcept
		{
			if constexpr (IsFloatingPoint<U>)
			{
				if (fabsf(static_cast<T>(scalar)) <= Epsilon<T>) return ZeroVector;
			}
			else
			{
				if (scalar == 0) return ZeroVector;
			}
			return { x / static_cast<T>(scalar), y / static_cast<T>(scalar) };
		}

		constexpr Vector2& operator+=(const Vector2& other) noexcept
		{
			x += other.x;
			y += other.y;
			return *this;
		}
		constexpr Vector2& operator-=(const Vector2& other) noexcept
		{
			x -= other.x;
			y -= other.y;
			return *this;
		}
		constexpr Vector2& operator*=(const Vector2& other) noexcept
		{
			x *= other.x;
			y *= other.y;
			return *this;
		}
	    Vector2& operator/=(const Vector2& other) noexcept
		{
			x = fabsf(other.x) > Epsilon<T> ? x / other.x : 0;
			y = fabsf(other.x) > Epsilon<T> ? y / other.y : 0;
			return *this;
		}

		template<Concept::Number U>
		constexpr Vector2& operator*=(U scalar) noexcept
		{
			x *= static_cast<T>(scalar);
			y *= static_cast<T>(scalar);
			return *this;
		}
		template<Concept::Number U>
		constexpr Vector2& operator/=(U scalar) noexcept
		{
			if constexpr (IsFloatingPoint<U>)
			{
				if (fabsf(static_cast<T>(scalar)) <= Epsilon<T>) return ZeroVector;
			}
			else
			{
				if (scalar == 0) return ZeroVector;
			}

			x /= static_cast<T>(scalar);
			y /= static_cast<T>(scalar);
			return *this;
		}


		template<Concept::FloatingPoint U>
		constexpr operator Vector2<U>() const noexcept
		{
			return Vector2<U>(static_cast<U>(x), static_cast<U>(y));
		}

	public:
		Vector2& Normalize() noexcept
		{
			T length = LengthSquared();
			if (length > Epsilon<T>)
			{
				length = sqrtf(length);
				x /= length;
				y /= length;
			}
			return *this;
		}
		MC_NODISCARD Vector2 Normalized() const noexcept
		{
			T length = LengthSquared();
			if (length > Epsilon<T>)
			{
				length = sqrtf(length);
				return { x / length, y / length };
			}
			return *this;
		}

		MC_NODISCARD constexpr Vector2 Lerp(const Vector2& vec, f32 t) const noexcept
        {
			return LERP(*this, vec, t);
		}

		MC_NODISCARD f64 GetAngleBetweenVectors(const Vector2& other) const noexcept
		{
			T length = LengthSquared() * other.LengthSquared();
			if (length <= Epsilon<T>) return 0;
			return acosf(static_cast<f64>(Dot(other)) / static_cast<f64>(sqrtf(length)));
		}

		MC_NODISCARD constexpr T Dot(const Vector2& other) const noexcept
		{
			return x * other.x + y * other.y;
		}
		MC_NODISCARD constexpr T Cross(const Vector2& other) const noexcept
        {
			return x * other.y - y * other.x;
		}

		MC_NODISCARD T Length() const noexcept
		{
			return static_cast<T>(sqrtf(SQUARE_SUM_2(x, y)));
		}
		MC_NODISCARD constexpr T LengthSquared() const noexcept
		{
			return SQUARE_SUM_2(x, y);
		}

		MC_NODISCARD constexpr Vector2 Project(const Vector2& other) const noexcept
		{
			T dot = Dot(other);
			T lenSquared = other.LengthSquared();
			if (lenSquared <= Epsilon<T>) return ZeroVector;
			return other * (dot / lenSquared);
		}
		MC_NODISCARD constexpr Vector2 Reflect(const Vector2& normal) const noexcept
		{
			return *this - normal * (2 * Dot(normal));
		}

	    Vector2& Abs() noexcept
		{
			x = fabsf(x);
			y = fabsf(y);
			return *this;
		}
		MC_NODISCARD Vector2 AsAbs() const noexcept
		{
			return { fabsf(x), fabsf(y) };
		}

		Vector2D Rotate(f32 angle) const noexcept
		{
			const f32 c = cosf(angle), s = sinf(angle);
			return { x * c - y * s, x * s + y * c };
		}
	};

	template <Concept::FloatingPoint T> inline constexpr Vector2<T> Vector2<T>::ZeroVector	{  T(0),  T(0)  };
	template <Concept::FloatingPoint T> inline constexpr Vector2<T> Vector2<T>::OneVector	{  T(1),  T(1)  };
	template <Concept::FloatingPoint T> inline constexpr Vector2<T> Vector2<T>::RightVector	{  T(1),  T(0)  };
	template <Concept::FloatingPoint T> inline constexpr Vector2<T> Vector2<T>::LeftVector	{  T(-1), T(0)  };
	template <Concept::FloatingPoint T> inline constexpr Vector2<T> Vector2<T>::UpVector	{  T(0),  T(1)  };
	template <Concept::FloatingPoint T> inline constexpr Vector2<T> Vector2<T>::DownVector	{  T(0),  T(-1) };
	template <Concept::FloatingPoint T> inline constexpr Vector2<T> Vector2<T>::xAxis		{  T(1),  T(0)  };
	template <Concept::FloatingPoint T> inline constexpr Vector2<T> Vector2<T>::yAxis		{  T(0),  T(1)  };
}

template<::MC::Concept::FloatingPoint T, ::MC::Concept::Number U>
constexpr ::MC::Math::Vector2<T> operator*(U scalar, const ::MC::Math::Vector2<T>& vec) noexcept
{
	return vec * scalar;
}

MC_FMT_FORMATTER(MC::Math::Vector2D32, "%f, %f", MC_FMT_FORMATTER_MEMBER(x), MC_FMT_FORMATTER_MEMBER(y));
MC_FMT_FORMATTER(MC::Math::Vector2D64, "%f, %f", MC_FMT_FORMATTER_MEMBER(x), MC_FMT_FORMATTER_MEMBER(y));