#pragma once
#include "MathCore.hpp"
#include "MoonCore/Core.hpp"
#include "Vector3.hpp"
#include "MoonCore/Utility/Format.hpp"

namespace MC::Math
{
	struct Matrix3x3
	{
	public:
		union
		{
			f32 Matrix[3][3];
			f32 a[3][3];
			struct
			{
				f32 a11, a12, a13;
				f32 a21, a22, a23;
				f32 a31, a32, a33;
			};
		};

	public:
		static const Matrix3x3 Identity;
		static const Matrix3x3 Empty;

	public:
		constexpr Matrix3x3() noexcept
			: a11(1.0f), a12(0.0f), a13(0.0f),
			a21(0.0f), a22(1.0f), a23(0.0f),
			a31(0.0f), a32(0.0f), a33(1.0f)
		{}

		constexpr Matrix3x3(f32 scalar) noexcept
			: a11(scalar), a12(0.0f), a13(0.0f),
			a21(0.0f), a22(scalar), a23(0.0f),
			a31(0.0f), a32(0.0f), a33(scalar)
		{}
		constexpr Matrix3x3(
			f32 m11, f32 m12, f32 m13,
			f32 m21, f32 m22, f32 m23,
			f32 m31, f32 m32, f32 m33) noexcept
			: a11(m11), a12(m12), a13(m13),
			a21(m21), a22(m22), a23(m23),
			a31(m31), a32(m32), a33(m33)
		{}
		constexpr Matrix3x3(
			const Vector3<f32>& row1,
			const Vector3<f32>& row2,
			const Vector3<f32>& row3) noexcept
			: a11(row1.x), a12(row1.y), a13(row1.z),
			a21(row2.x), a22(row2.y), a23(row2.z),
			a31(row3.x), a32(row3.y), a33(row3.z)
		{}

		constexpr Matrix3x3(const Matrix3x3&) noexcept = default;
		constexpr Matrix3x3(Matrix3x3&&) noexcept = default;
		constexpr Matrix3x3& operator=(const Matrix3x3&) noexcept = default;
		constexpr Matrix3x3& operator=(Matrix3x3&&) noexcept = default;
		constexpr ~Matrix3x3() = default;

	public:
		bool operator==(const Matrix3x3& other) const noexcept
		{
			return fabsf(a11 - other.a11) < Epsilon<f32> &&
				fabsf(a12 - other.a12) < Epsilon<f32> &&
				fabsf(a13 - other.a13) < Epsilon<f32> &&
				fabsf(a21 - other.a21) < Epsilon<f32> &&
				fabsf(a22 - other.a22) < Epsilon<f32> &&
				fabsf(a23 - other.a23) < Epsilon<f32> &&
				fabsf(a31 - other.a31) < Epsilon<f32> &&
				fabsf(a32 - other.a32) < Epsilon<f32> &&
				fabsf(a33 - other.a33) < Epsilon<f32>;
		}
		constexpr Matrix3x3 operator+(const Matrix3x3& other) const noexcept
		{
			return {
				a11 + other.a11, a12 + other.a12, a13 + other.a13,
				a21 + other.a21, a22 + other.a22, a23 + other.a23,
				a31 + other.a31, a32 + other.a32, a33 + other.a33
			};
		}
		constexpr Matrix3x3 operator-(const Matrix3x3& other) const noexcept
		{
			return {
				a11 - other.a11, a12 - other.a12, a13 - other.a13,
				a21 - other.a21, a22 - other.a22, a23 - other.a23,
				a31 - other.a31, a32 - other.a32, a33 - other.a33
			};
		}

		constexpr Matrix3x3 operator*(const Matrix3x3& other) const noexcept
		{
			return {
				a11 * other.a11 + a12 * other.a21 + a13 * other.a31,
				a11 * other.a12 + a12 * other.a22 + a13 * other.a32,
				a11 * other.a13 + a12 * other.a23 + a13 * other.a33,

				a21 * other.a11 + a22 * other.a21 + a23 * other.a31,
				a21 * other.a12 + a22 * other.a22 + a23 * other.a32,
				a21 * other.a13 + a22 * other.a23 + a23 * other.a33,

				a31 * other.a11 + a32 * other.a21 + a33 * other.a31,
				a31 * other.a12 + a32 * other.a22 + a33 * other.a32,
				a31 * other.a13 + a32 * other.a23 + a33 * other.a33
			};
		}
		constexpr Vector3<f32> operator*(const Vector3<f32>& other) const noexcept
		{
			return {
				other.x * a11 + other.y * a21 + other.z * a31,
				other.x * a12 + other.y * a22 + other.z * a32,
				other.x * a13 + other.y * a23 + other.z * a33,
			};
		}
		template <Concept::Number T>
		constexpr Matrix3x3 operator*(T scalar) const noexcept
		{
			return {
				a11 * scalar, a12 * scalar, a13 * scalar,
				a21 * scalar, a22 * scalar, a23 * scalar,
				a31 * scalar, a32 * scalar, a33 * scalar
			};
		}

		constexpr Matrix3x3& operator+=(const Matrix3x3& other) noexcept
		{
			a11 += other.a11; a12 += other.a12; a13 += other.a13;
			a21 += other.a21; a22 += other.a22; a23 += other.a23;
			a31 += other.a31; a32 += other.a32; a33 += other.a33;
			return *this;
		}
		constexpr Matrix3x3& operator-=(const Matrix3x3& other) noexcept
		{
			a11 -= other.a11; a12 -= other.a12; a13 -= other.a13;
			a21 -= other.a21; a22 -= other.a22; a23 -= other.a23;
			a31 -= other.a31; a32 -= other.a32; a33 -= other.a33;
			return *this;
		}
		constexpr Matrix3x3& operator*=(const Matrix3x3& other) noexcept
		{
			return *this = *this * other;
		}
		template <Concept::Number T>
		constexpr Matrix3x3& operator*=(T scalar) noexcept
		{
			a11 *= scalar; a12 *= scalar; a13 *= scalar;
			a21 *= scalar; a22 *= scalar; a23 *= scalar;
			a31 *= scalar; a32 *= scalar; a33 *= scalar;
			return *this;
		}

	public:
		constexpr f32 Determinant() const noexcept
		{
			return (a11 * a22 * a33 + a21 * a32 * a13 + a31 * a12 * a23)
				- (a13 * a22 * a31 + a23 * a32 * a11 + a33 * a12 * a21);
		}

		Matrix3x3& Inverse() noexcept
		{
			f32 determinant = Determinant();
			if (fabs(determinant) < Epsilon<f32>) return *this;

			determinant = 1.0f / determinant;

			const f32 m11 = a11, m12 = a12, m13 = a13;
			const f32 m21 = a21, m22 = a22, m23 = a23;
			const f32 m31 = a31, m32 = a32, m33 = a33;

			a11 = determinant * (m22 * m33 - m23 * m32);
			a12 = determinant * -(m12 * m33 - m13 * m32);
			a13 = determinant * (m12 * m23 - m13 * m22);

			a21 = determinant * -(m21 * m33 - m23 * m31);
			a22 = determinant * (m11 * m33 - m13 * m31);
			a23 = determinant * -(m11 * m23 - m13 * m21);

			a31 = determinant * (m21 * m32 - m22 * m31);
			a32 = determinant * -(m11 * m32 - m12 * m31);
			a33 = determinant * (m11 * m22 - m12 * m21);

			return *this;
		}
		Matrix3x3 Inversed() const noexcept
		{
			Matrix3x3 result = *this;
			return result.Inverse();
		}
	    
	    constexpr Matrix3x3& InverseAffine() noexcept
		{
			Swap(a12, a21);
			Swap(a13, a31);
			Swap(a23, a32);
			return *this;
		}
	    constexpr Matrix3x3 InversedAffine() const noexcept
		{
			Matrix3x3 result = *this;
			return result.InverseAffine();
		}
		
		constexpr Matrix3x3& Transpose() noexcept
		{
			Swap(a12, a21);
			Swap(a13, a31);
			Swap(a23, a32);
			return *this;
		}
		constexpr Matrix3x3 Transposed() const noexcept
		{
			Matrix3x3 result = *this;
			return result.Transpose();
		}

	    constexpr Matrix3x3 Adjoint() const noexcept
		{
			return {
				MAT2x2_DET(a22, a23, a32, a33),		-MAT2x2_DET(a12, a13, a32, a33),	MAT2x2_DET(a12, a13, a22, a23),
				-MAT2x2_DET(a21, a23, a31, a33),	MAT2x2_DET(a11, a13, a31, a33),		-MAT2x2_DET(a11, a13, a21, a23),
			    MAT2x2_DET(a21, a22, a31, a32),		-MAT2x2_DET(a11, a12, a31, a32),	MAT2x2_DET(a11, a12, a21, a22)
			};
		}
		Matrix3x3 Abs() const noexcept
		{
			return {
				fabsf(a11), fabsf(a12), fabsf(a13),
				fabsf(a21), fabsf(a22), fabsf(a23),
				fabsf(a31), fabsf(a32), fabsf(a33)
			};
		}
	};

	constexpr Matrix3x3 Matrix3x3::Identity = Matrix3x3(1.f);
	constexpr Matrix3x3 Matrix3x3::Empty = Matrix3x3(0.f);
}

constexpr MC::Math::Matrix3x3 operator*(const f32& scalar, const MC::Math::Matrix3x3& mat) noexcept
{
	return mat * scalar;
}

constexpr MC::Math::Vector3<f32> operator*(const MC::Math::Vector3<f32>& vector, const MC::Math::Matrix3x3& matrix) noexcept
{
	return {
		vector.x * matrix.a11 + vector.y * matrix.a21 + vector.z * matrix.a31,
		vector.x * matrix.a12 + vector.y * matrix.a22 + vector.z * matrix.a32,
		vector.x * matrix.a13 + vector.y * matrix.a23 + vector.z * matrix.a33,
	};
}

MC_FMT_FORMATTER(MC::Math::Matrix3x3, "Row 0: %f, %f, %f\n"
                                                 "Row 1: %f, %f, %f\n"
                                                 "Row 2: %f, %f, %f\n",
	MC_FMT_FORMATTER_VALUE(a11), MC_FMT_FORMATTER_VALUE(a12), MC_FMT_FORMATTER_VALUE(a13),
	MC_FMT_FORMATTER_VALUE(a21), MC_FMT_FORMATTER_VALUE(a22), MC_FMT_FORMATTER_VALUE(a23),
	MC_FMT_FORMATTER_VALUE(a31), MC_FMT_FORMATTER_VALUE(a32), MC_FMT_FORMATTER_VALUE(a33)
);