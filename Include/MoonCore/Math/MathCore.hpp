#pragma once
#include "MoonCore/Types.hpp"
#include "MathMacros.hpp"
#include "MoonCore/Utility/Algorithm.hpp"
#include "MoonCore/Concept.hpp"
#include "MoonCore/TypeTraits.hpp"
#include "MoonCore/Logging/Logger.hpp"
#include <math.h>

namespace MC::Concept
{
	template<typename T, typename F>
	concept Lerpable = Concept::FloatingPoint<F> && requires(T a, T b, F t)
	{
		{ a + (b - a) * t } -> ConvertibleTo<T>;
	};
}

namespace MC::Math
{
	namespace Constants
	{
		inline constexpr f64 Pi = PI;
	}

	template <Concept::FloatingPoint T> static constexpr T Epsilon = NumericLimit<T>::Epsilon;

	struct Matrix3x3;
	struct Matrix4x4;

	struct Quaternion;

	template<Concept::FloatingPoint T>
	struct Vector2;

	template<Concept::FloatingPoint T>
	struct Vector3;

	template<Concept::FloatingPoint T>
	struct Vector4;

	struct Color3;
	struct Color4;

	class Transform;

	template<Concept::Number T>
	struct Resolution2D;

	template<Concept::Number T>
	struct Resolution3D;

	struct Rect2D;

	// Vector2
	typedef Vector2<f32> Vector2D32;
	typedef Vector2<f64> Vector2D64;
	typedef Vector2D32 Vector2D;

	// Vector3
	typedef Vector3<f32> Vector3D32;
	typedef Vector3<f64> Vector3D64;
	typedef Vector3D32 Vector3D;

	// Vector4
	typedef Vector4<f32> Vector4D32;
	typedef Vector4<f64> Vector4D64;
	typedef Vector4D32 Vector4D;

	template<typename T, Concept::FloatingPoint FPoint>
		requires Concept::Lerpable<T, FPoint>
    static constexpr T Lerp(T a, T b, FPoint t) noexcept
	{
		return static_cast<T>(a + (b - a) * t);
	}

	template <Concept::FloatingPoint T>
	static constexpr T ToDegrees(T radians) noexcept
	{
		return static_cast<T>(radians / (T(180) / T(PI)));
	}

	template <Concept::FloatingPoint T>
	static constexpr T ToRadians(T degrees) noexcept
	{
		return static_cast<T>(degrees * (T(PI) / T(180)));
	}
}