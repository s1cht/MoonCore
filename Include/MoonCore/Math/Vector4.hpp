#pragma once 
#include "MoonCore/Math/MathCore.hpp"
#include "MoonCore/Math/Vector2.hpp"
#include "MoonCore/Math/Vector3.hpp"

// TODO: 128-bit SIMD Vector4

namespace MC::Math
{
    template<Concept::FloatingPoint T>
	struct Vector4
	{
	public:
		union
		{
			struct { T x, y, z, w; };
			struct { T X, Y, Z, W; };
			T XYZW[4];
		};

    public:
		static const Vector4 ZeroVector;
		static const Vector4 OneVector;
		static const Vector4 xAxis;
		static const Vector4 yAxis;
		static const Vector4 zAxis;
		static const Vector4 wAxis;

	public:
		constexpr Vector4() noexcept
			: x(0), y(0), z(0), w(0)
		{}
		explicit constexpr Vector4(T scalar) noexcept
			: x(scalar), y(scalar), z(scalar), w(scalar)
		{}
		constexpr Vector4(T xValue, T yValue, T zValue, T wValue) noexcept
			: x(xValue), y(yValue), z(zValue), w(wValue)
		{}
		constexpr Vector4(T vector[4]) noexcept
			: x(vector[0]), y(vector[1]), z(vector[2]), w(vector[3])
		{}
		constexpr Vector4(const Vector2<T>& xy, const Vector2<T>& zw) noexcept
			: x(xy.x), y(xy.y), z(zw.x), w(zw.y)
		{}
		constexpr Vector4(const Vector3<T>& vec, T _w) noexcept
			: x(vec.x), y(vec.y), z(vec.z), w(_w)
		{}

		constexpr Vector4(const Vector4&) noexcept = default;
		constexpr Vector4(Vector4&&) noexcept = default;
		constexpr Vector4& operator=(const Vector4&) noexcept = default;
		constexpr Vector4& operator=(Vector4&&) noexcept = default;

		constexpr ~Vector4() = default;

	public:
		bool operator==(const Vector4& other) const noexcept
		{
			return abs(x - other.x) < Epsilon<T> &&
				   abs(y - other.y) < Epsilon<T> &&
				   abs(z - other.z) < Epsilon<T> &&
				   abs(w - other.w) < Epsilon<T>;
		}

		constexpr Vector4 operator-() const noexcept
		{
			return { -x, -y, -z, -w };
		}

		constexpr Vector4 operator+(const Vector4& other) const noexcept
		{
			return { x + other.x, y + other.y, z + other.z, w + other.w };
		}
		constexpr Vector4 operator-(const Vector4& other) const noexcept
		{
			return { x - other.x, y - other.y, z - other.z, w - other.w };
		}
		constexpr Vector4 operator*(const Vector4& other) const noexcept
		{
			return { x * other.x, y * other.y, z * other.z, w * other.w };
		}
        Vector4 operator/(const Vector4& other) const noexcept
		{
			return {
				abs(other.x) > Epsilon<T> ? x / other.x : 0,
				abs(other.y) > Epsilon<T> ? y / other.y : 0,
				abs(other.z) > Epsilon<T> ? z / other.z : 0,
				abs(other.w) > Epsilon<T> ? w / other.w : 0
			};
		}

		template<Concept::Number U>
		constexpr Vector4 operator*(U scalar) const noexcept
		{
			return { x * static_cast<T>(scalar), y * static_cast<T>(scalar), z * static_cast<T>(scalar), w * static_cast<T>(scalar) };
		}

		template<Concept::Number U>
		Vector4 operator/(U scalar) const noexcept
        {
			if constexpr (IsFloatingPoint<U>)
			{
				if (abs(static_cast<T>(scalar)) <= Epsilon<T>) return ZeroVector;
			}
			else
			{
				if (scalar == 0) return ZeroVector;
			}
			return { x / static_cast<T>(scalar), y / static_cast<T>(scalar), z / static_cast<T>(scalar), w / static_cast<T>(scalar) };
		}

		constexpr Vector4& operator+=(const Vector4& other) noexcept
        {
			x += other.x;
			y += other.y;
			z += other.z;
			w += other.w;
			return *this;
		}
		constexpr Vector4& operator-=(const Vector4& other) noexcept
        {
			x -= other.x;
			y -= other.y;
			z -= other.z;
			w -= other.w;
			return *this;
		}

		constexpr Vector4& operator*=(const Vector4& other) noexcept
		{
			x *= other.x;
			y *= other.y;
			z *= other.z;
			w *= other.w;
			return *this;
		}
        Vector4& operator/=(const Vector4& other) noexcept
        {
			x = abs(other.x) > Epsilon<T> ? x / other.x : 0;
			y = abs(other.y) > Epsilon<T> ? y / other.y : 0;
			z = abs(other.z) > Epsilon<T> ? z / other.z : 0;
			w = abs(other.w) > Epsilon<T> ? w / other.w : 0;
			return *this;
		}
		
		template<Concept::Number U>
		constexpr Vector4& operator*=(U scalar) noexcept
        {
			x *= static_cast<T>(scalar);
			y *= static_cast<T>(scalar);
			z *= static_cast<T>(scalar);
			w *= static_cast<T>(scalar);
			return *this;
		}
		template<Concept::Number U>
        Vector4& operator/=(U scalar) noexcept
        {
			if constexpr (IsFloatingPoint<U>)
			{
				if (abs(static_cast<T>(scalar)) <= Epsilon<T>) return *this;
			}
			else
			{
				if (scalar == 0) return *this;
			}

			x /= static_cast<T>(scalar);
			y /= static_cast<T>(scalar);
			z /= static_cast<T>(scalar);
			w /= static_cast<T>(scalar);
			return *this;
		}

	public:
		Vector4& Normalize() noexcept
		{
			T length = LengthSquared();
			if (length > 0)
			{
				length = sqrt(length);
				x /= length;
				y /= length;
				z /= length;
				w /= length;
			}
			return *this;
		}
		MC_NODISCARD Vector4 Normalized() const noexcept
		{
			T length = LengthSquared();
		    if (length > Epsilon<T>)
			{
				length = sqrt(length);
				return { x / length, y / length, z / length, w / length };
			}
			return ZeroVector;
		}
		
		MC_NODISCARD constexpr Vector4 Lerp(const Vector4& vec, f32 t) const noexcept
		{
			return LERP(*this, vec, t);
		}
		MC_NODISCARD constexpr Vector4 Cross(const Vector4& other) const noexcept
		{
			return{
				y * other.z - z * other.y,
				z * other.x - x * other.z,
				x * other.y - y * other.x,
				0
			};
		}
		MC_NODISCARD f64 GetAngleBetweenVectors(const Vector4& other) const noexcept
		{
			T length = LengthSquared() * other.LengthSquared();
			if (length <= Epsilon<T>) return 0;
			return acos(static_cast<f64>(Dot(other)) / static_cast<f64>(sqrt(length)));
		}

		MC_NODISCARD constexpr T Dot(const Vector4& other) const noexcept
		{
			return x * other.x + y * other.y + z * other.z + w * other.w;
		}
		MC_NODISCARD inline T Length() const noexcept
		{
			return static_cast<T>(sqrt(static_cast<double>(SQUARE_SUM_4(x, y, z, w))));
		}
		MC_NODISCARD constexpr T LengthSquared() const noexcept
		{
			return SQUARE_SUM_4(x, y, z, w);
		}

		MC_NODISCARD constexpr Vector4 Project(const Vector4& other) const noexcept
	    {
			T lengthSquared = other.LengthSquared();
			if (lengthSquared <= Epsilon<T>) return ZeroVector;
			return other * (Dot(other) / lengthSquared);
	    }
		MC_NODISCARD constexpr Vector4 Reflect(const Vector4& normal) const noexcept
		{
			return *this - normal * (2 * Dot(normal));
		}
	};

	template <Concept::FloatingPoint T> inline constexpr Vector4<T> Vector4<T>::ZeroVector{ T(0), T(0), T(0), T(0) };
	template <Concept::FloatingPoint T> inline constexpr Vector4<T> Vector4<T>::OneVector { T(1), T(1), T(1), T(1) };
	template <Concept::FloatingPoint T> inline constexpr Vector4<T> Vector4<T>::xAxis	 { T(1), T(0), T(0), T(0) };
	template <Concept::FloatingPoint T> inline constexpr Vector4<T> Vector4<T>::yAxis	 { T(0), T(1), T(0), T(0) };
	template <Concept::FloatingPoint T> inline constexpr Vector4<T> Vector4<T>::zAxis	 { T(0), T(0), T(1), T(0) };
	template <Concept::FloatingPoint T> inline constexpr Vector4<T> Vector4<T>::wAxis	 { T(0), T(0), T(0), T(1) };
}

template<::MC::Concept::FloatingPoint T, ::MC::Concept::Number U>
constexpr ::MC::Math::Vector4<T> operator*(U scalar, const ::MC::Math::Vector4<T>& vec) noexcept
{
	return vec * scalar;
}

MC_FMT_FORMATTER(MC::Math::Vector4D32, "%f, %f, %f, %f", MC_FMT_FORMATTER_MEMBER(x), MC_FMT_FORMATTER_MEMBER(y), MC_FMT_FORMATTER_MEMBER(z), MC_FMT_FORMATTER_MEMBER(w));
MC_FMT_FORMATTER(MC::Math::Vector4D64, "%f, %f, %f, %f", MC_FMT_FORMATTER_MEMBER(x), MC_FMT_FORMATTER_MEMBER(y), MC_FMT_FORMATTER_MEMBER(z), MC_FMT_FORMATTER_MEMBER(w));