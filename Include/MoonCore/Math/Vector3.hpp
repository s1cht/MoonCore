#pragma once 
#include "MoonCore/Math/MathCore.hpp"
#include "MoonCore/Math/Vector2.hpp"

namespace MC::Math
{
	template<Concept::FloatingPoint T>
	struct Vector3
	{
	public:
		union
		{
			struct { T x, y, z; };
			struct { T X, Y, Z; };
			T XYZ[3];
		};

	public:
		static const Vector3 ZeroVector;
		static const Vector3 OneVector;
        static const Vector3 RightVector;
        static const Vector3 LeftVector;
        static const Vector3 UpVector;
		static const Vector3 DownVector;
        static const Vector3 BackwardVector;
        static const Vector3 ForwardVector;
        static const Vector3 xAxis;
		static const Vector3 yAxis;
		static const Vector3 zAxis;

	public:
		explicit constexpr Vector3() noexcept
			: x(0), y(0), z(0)
		{}
		explicit constexpr Vector3(T scalar) noexcept
			: x(scalar), y(scalar), z(scalar)
		{}
		constexpr Vector3(T xValue, T yValue, T zValue) noexcept
			: x(xValue), y(yValue), z(zValue)
		{}
		constexpr Vector3(T vector[3]) noexcept
			: x(vector[0]), y(vector[1]), z(vector[2])
		{}
		constexpr Vector3(const Vector2<T>& vec, T zValue) noexcept
			: x(vec.x), y(vec.y), z(zValue)
		{}

		constexpr Vector3(const Vector3&) noexcept = default;
		constexpr Vector3(Vector3&&) noexcept = default;
		constexpr Vector3& operator=(const Vector3&) noexcept = default;
		constexpr Vector3& operator=(Vector3&&) noexcept = default;
	
		constexpr ~Vector3() = default;

	public:
	    bool operator==(const Vector3& other) const noexcept 
	    {
			return abs(x - other.x) < Epsilon<T> &&
				   abs(y - other.y) < Epsilon<T> &&
				   abs(z - other.z) < Epsilon<T>;
		}

		constexpr Vector3 operator-() const noexcept
		{
			return { -x, -y, -z };
		}

		constexpr Vector3 operator+(const Vector3& other) const noexcept
		{
			return { x + other.x, y + other.y, z + other.z };
		}
		constexpr Vector3 operator-(const Vector3& other) const noexcept
		{
	        return { x - other.x, y - other.y, z - other.z };
		}
		constexpr Vector3 operator*(const Vector3& other) const noexcept
		{
	        return { x * other.x, y * other.y, z * other.z };
		}
		Vector3 operator/(const Vector3& other) const noexcept
		{
			return {
				abs(other.x) > Epsilon<T> ? x / other.x : 0,
				abs(other.y) > Epsilon<T> ? y / other.y : 0,
				abs(other.z) > Epsilon<T> ? z / other.z : 0
			};
		}

		template<Concept::Number U>
		constexpr Vector3 operator*(U scalar) const noexcept
		{
			return { x * static_cast<T>(scalar), y * static_cast<T>(scalar), z * static_cast<T>(scalar) };
		}

		template<Concept::Number U>
		Vector3 operator/(U scalar) const noexcept
		{
			if constexpr (IsFloatingPoint<U>)
			{
				if (abs(static_cast<T>(scalar)) <= Epsilon<T>) return ZeroVector;
			}
			else
			{
				if (scalar == 0) return ZeroVector;
			}
			return { x / static_cast<T>(scalar), y / static_cast<T>(scalar), z / static_cast<T>(scalar) };
		}

		constexpr Vector3& operator+=(const Vector3& other) noexcept
		{
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}
		constexpr Vector3& operator-=(const Vector3& other) noexcept
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}
		constexpr Vector3& operator*=(const Vector3& other) noexcept
		{
			x *= other.x;
			y *= other.y;
			z *= other.z;
			return *this;
		}
		Vector3& operator/=(const Vector3& other) noexcept
		{
			x = abs(other.x) > Epsilon<T> ? x / other.x : 0;
			y = abs(other.x) > Epsilon<T> ? y / other.y : 0;
			z = abs(other.x) > Epsilon<T> ? z / other.z : 0;
			return *this;
		}

		template<Concept::Number U>
		constexpr Vector3& operator*=(U scalar) noexcept
		{
			x *= static_cast<T>(scalar);
			y *= static_cast<T>(scalar);
			z *= static_cast<T>(scalar);
			return *this;
		}
		template<Concept::Number U>
		constexpr Vector3& operator/=(U scalar) noexcept
		{
			if constexpr (IsFloatingPoint<U>)
			{
				if (abs(static_cast<T>(scalar)) <= Epsilon<T>) return ZeroVector;
			}
			else
			{
				if (scalar == 0) return ZeroVector;
			}

			x /= static_cast<T>(scalar);
			y /= static_cast<T>(scalar);
			z /= static_cast<T>(scalar);
			return *this;
		}

		template<Concept::FloatingPoint U>
		constexpr operator Vector3<U>() const noexcept
		{
			return Vector3<U>(static_cast<U>(x), static_cast<U>(y), static_cast<U>(z));
		}

	public:
		Vector3& Normalize() noexcept
		{
			T length = LengthSquared();
			if (length > Epsilon<T>)
			{
				length = sqrt(length);
				x /= length;
				y /= length;
				z /= length;
			}
			return *this;
		}
		MC_NODISCARD Vector3 Normalized() const noexcept
		{
			T length = LengthSquared();
			if (length > Epsilon<T>)
			{
				length = static_cast<T>(sqrt(length));
				return { x / length, y / length, z / length };
			}
			return ZeroVector;
		}

		MC_NODISCARD constexpr Vector3 Lerp(const Vector3& vec, f32 t) const noexcept
		{
			return LERP(*this, vec, t);
		}

		MC_NODISCARD  f64 GetAngleBetweenVectors(const Vector3& other) const noexcept
		{
			T length = LengthSquared() * other.LengthSquared();
			if (length <= Epsilon<T>) return 0;
			return acos(static_cast<f64>(Dot(other)) / static_cast<f64>(sqrt(length)));
		}

		MC_NODISCARD constexpr T Dot(const Vector3& other) const noexcept
		{
			return x * other.x + y * other.y + z * other.z;
		}
		MC_NODISCARD constexpr Vector3 Cross(const Vector3& other) const noexcept
		{
			return {
				y * other.z - z * other.y,
				z * other.x - x * other.z,
				x * other.y - y * other.x
			};
		}

		MC_NODISCARD T Length() const noexcept
		{
			return static_cast<T>(sqrt(SQUARE_SUM_3(x, y, z)));
		}
		MC_NODISCARD constexpr T LengthSquared() const noexcept
		{
			return SQUARE_SUM_3(x, y, z);
		}

		MC_NODISCARD constexpr Vector3 Project(const Vector3& other) const noexcept
		{
			T dot = Dot(other);
			T lenSquared = other.LengthSquared();
			if (lenSquared <= Epsilon<T>) return ZeroVector;
			return other * (dot / lenSquared);
		}
		MC_NODISCARD constexpr Vector3 Reflect(const Vector3& normal) const noexcept
		{
			return *this - normal * (2 * Dot(normal));
		}

		Vector3& Abs() noexcept
		{
			x = abs(x);
			y = abs(y);
			z = abs(z);
			return *this;
		}
		MC_NODISCARD Vector3 AsAbs() const noexcept
		{
			return { abs(x), abs(y), abs(z) };
		}
	};

	template <Concept::FloatingPoint T> inline constexpr Vector3<T> Vector3<T>::ZeroVector{ T(0),  T(0),   T(0) };
	template <Concept::FloatingPoint T> inline constexpr Vector3<T> Vector3<T>::OneVector{ T(1),  T(1),   T(1) };
	template <Concept::FloatingPoint T> inline constexpr Vector3<T> Vector3<T>::RightVector{ T(1),  T(0),   T(0) };
	template <Concept::FloatingPoint T> inline constexpr Vector3<T> Vector3<T>::LeftVector{ T(-1), T(0),   T(0) };
	template <Concept::FloatingPoint T> inline constexpr Vector3<T> Vector3<T>::UpVector{ T(0),  T(1),   T(0) };
	template <Concept::FloatingPoint T> inline constexpr Vector3<T> Vector3<T>::DownVector{ T(0),  T(-1),  T(0) };
	template <Concept::FloatingPoint T> inline constexpr Vector3<T> Vector3<T>::BackwardVector{ T(0),  T(0),   T(1) };
	template <Concept::FloatingPoint T> inline constexpr Vector3<T> Vector3<T>::ForwardVector{ T(0),  T(0),   T(-1) };
	template <Concept::FloatingPoint T> inline constexpr Vector3<T> Vector3<T>::xAxis{ T(1),  T(0),   T(0) };
	template <Concept::FloatingPoint T> inline constexpr Vector3<T> Vector3<T>::yAxis{ T(0),  T(1),   T(0) };
	template <Concept::FloatingPoint T> inline constexpr Vector3<T> Vector3<T>::zAxis{ T(0),  T(0),   T(1) };
}

template<::MC::Concept::FloatingPoint T, ::MC::Concept::Number U>
constexpr ::MC::Math::Vector3<T> operator*(U scalar, const ::MC::Math::Vector3<T>& vec) noexcept
{
	return vec * scalar;
}

MC_FMT_FORMATTER(MC::Math::Vector3D32, "%f, %f, %f", MC_FMT_FORMATTER_MEMBER(x), MC_FMT_FORMATTER_MEMBER(y), MC_FMT_FORMATTER_MEMBER(z));
MC_FMT_FORMATTER(MC::Math::Vector3D64, "%f, %f, %f", MC_FMT_FORMATTER_MEMBER(x), MC_FMT_FORMATTER_MEMBER(y), MC_FMT_FORMATTER_MEMBER(z));