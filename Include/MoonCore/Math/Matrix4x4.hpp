#pragma once
#include "MathCore.hpp"
#include "MoonCore/Core.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"
#include "Quaternion.hpp"

// TODO: 256-bit SIMD Matrix

namespace MC::Math
{
	struct Matrix4x4
	{
	public:
		union
		{
			f32 Matrix[4][4];
			f32 a[4][4];
			struct
			{
				f32 a11, a12, a13, a14;
				f32 a21, a22, a23, a24;
				f32 a31, a32, a33, a34;
				f32 a41, a42, a43, a44;
			};
		};

	public:
		constexpr Matrix4x4() noexcept
			: a11(1.0f), a12(0.0f), a13(0.0f), a14(0.0f),
			a21(0.0f), a22(1.0f), a23(0.0f), a24(0.0f),
			a31(0.0f), a32(0.0f), a33(1.0f), a34(0.0f),
			a41(0.0f), a42(0.0f), a43(0.0f), a44(1.0f)
		{}
		constexpr Matrix4x4(f32 m11, f32 m12, f32 m13, f32 m14,
			f32 m21, f32 m22, f32 m23, f32 m24,
			f32 m31, f32 m32, f32 m33, f32 m34,
			f32 m41, f32 m42, f32 m43, f32 m44) noexcept
			: a11(m11), a12(m12), a13(m13), a14(m14),
			a21(m21), a22(m22), a23(m23), a24(m24),
			a31(m31), a32(m32), a33(m33), a34(m34),
			a41(m41), a42(m42), a43(m43), a44(m44)
		{}
		constexpr Matrix4x4(f32 scalar) noexcept
			: a11(scalar), a12(0.0f), a13(0.0f), a14(0.0f),
			a21(0.0f), a22(scalar), a23(0.0f), a24(0.0f),
			a31(0.0f), a32(0.0f), a33(scalar), a34(0.0f),
			a41(0.0f), a42(0.0f), a43(0.0f), a44(scalar)
		{}
		constexpr Matrix4x4(const Vector4<f32>& row1,
			const Vector4<f32>& row2,
			const Vector4<f32>& row3,
			const Vector4<f32>& row4)
			: a11(row1.x), a12(row1.y), a13(row1.z), a14(row1.w),
			a21(row2.x), a22(row2.y), a23(row2.z), a24(row2.w),
			a31(row3.x), a32(row3.y), a33(row3.z), a34(row3.w),
			a41(row4.x), a42(row4.y), a43(row4.z), a44(row4.w)
		{}

		constexpr Matrix4x4(const Matrix4x4&) noexcept = default;
		constexpr Matrix4x4(Matrix4x4&&) noexcept = default;
		constexpr Matrix4x4& operator=(const Matrix4x4&) noexcept = default;
		constexpr Matrix4x4& operator=(Matrix4x4&&) noexcept = default;
		constexpr ~Matrix4x4() = default;
	
	public:
		static constexpr Matrix4x4 Empty() noexcept
		{
			return { 0.f };
		}

		static constexpr Matrix4x4 Identity() noexcept
		{
			return { 1.f };
		}

	public:
		bool operator==(const Matrix4x4& other) const noexcept
		{
			return fabsf(a11 - other.a11) < Epsilon<f32> &&
				fabsf(a12 - other.a12) < Epsilon<f32> &&
				fabsf(a13 - other.a13) < Epsilon<f32> &&
				fabsf(a14 - other.a14) < Epsilon<f32> &&
				fabsf(a21 - other.a21) < Epsilon<f32> &&
				fabsf(a22 - other.a22) < Epsilon<f32> &&
				fabsf(a23 - other.a23) < Epsilon<f32> &&
				fabsf(a24 - other.a24) < Epsilon<f32> &&
				fabsf(a31 - other.a31) < Epsilon<f32> &&
				fabsf(a32 - other.a32) < Epsilon<f32> &&
				fabsf(a33 - other.a33) < Epsilon<f32> &&
				fabsf(a34 - other.a34) < Epsilon<f32> &&
				fabsf(a41 - other.a41) < Epsilon<f32> &&
				fabsf(a42 - other.a42) < Epsilon<f32> &&
				fabsf(a43 - other.a43) < Epsilon<f32> &&
				fabsf(a44 - other.a44) < Epsilon<f32>;
		}

		constexpr Matrix4x4 operator+(const Matrix4x4& other) const noexcept
		{
			return {
				a11 + other.a11, a12 + other.a12, a13 + other.a13, a14 + other.a14,
				a21 + other.a21, a22 + other.a22, a23 + other.a23, a24 + other.a24,
				a31 + other.a31, a32 + other.a32, a33 + other.a33, a34 + other.a34,
				a41 + other.a41, a42 + other.a42, a43 + other.a43, a44 + other.a44
			};
		}
		constexpr Matrix4x4 operator-(const Matrix4x4& other) const noexcept
		{
			return {
				a11 - other.a11, a12 - other.a12, a13 - other.a13, a14 - other.a14,
				a21 - other.a21, a22 - other.a22, a23 - other.a23, a24 - other.a24,
				a31 - other.a31, a32 - other.a32, a33 - other.a33, a34 - other.a34,
				a41 - other.a41, a42 - other.a42, a43 - other.a43, a44 - other.a44
			};
		}
		constexpr Matrix4x4 operator*(const Matrix4x4& other) const noexcept
		{
			return {
				a11 * other.a11 + a12 * other.a21 + a13 * other.a31 + a14 * other.a41,
				a11 * other.a12 + a12 * other.a22 + a13 * other.a32 + a14 * other.a42,
				a11 * other.a13 + a12 * other.a23 + a13 * other.a33 + a14 * other.a43,
				a11 * other.a14 + a12 * other.a24 + a13 * other.a34 + a14 * other.a44,

				a21 * other.a11 + a22 * other.a21 + a23 * other.a31 + a24 * other.a41,
				a21 * other.a12 + a22 * other.a22 + a23 * other.a32 + a24 * other.a42,
				a21 * other.a13 + a22 * other.a23 + a23 * other.a33 + a24 * other.a43,
				a21 * other.a14 + a22 * other.a24 + a23 * other.a34 + a24 * other.a44,

				a31 * other.a11 + a32 * other.a21 + a33 * other.a31 + a34 * other.a41,
				a31 * other.a12 + a32 * other.a22 + a33 * other.a32 + a34 * other.a42,
				a31 * other.a13 + a32 * other.a23 + a33 * other.a33 + a34 * other.a43,
				a31 * other.a14 + a32 * other.a24 + a33 * other.a34 + a34 * other.a44,

				a41 * other.a11 + a42 * other.a21 + a43 * other.a31 + a44 * other.a41,
				a41 * other.a12 + a42 * other.a22 + a43 * other.a32 + a44 * other.a42,
				a41 * other.a13 + a42 * other.a23 + a43 * other.a33 + a44 * other.a43,
				a41 * other.a14 + a42 * other.a24 + a43 * other.a34 + a44 * other.a44
			};
		}
		constexpr Vector4<f32> operator*(const Vector4<f32>& other) const noexcept
		{
			return {
				a11 * other.x + a12 * other.y + a13 * other.z + a14 * other.w,
				a21 * other.x + a22 * other.y + a23 * other.z + a24 * other.w,
				a31 * other.x + a32 * other.y + a33 * other.z + a34 * other.w,
				a41 * other.x + a42 * other.y + a43 * other.z + a44 * other.w
			};
		}
		template <Concept::Number T>
		constexpr Matrix4x4 operator*(T scalar) const noexcept
		{
			return {
				a11 * scalar, a12 * scalar, a13 * scalar, a14 * scalar,
				a21 * scalar, a22 * scalar, a23 * scalar, a24 * scalar,
				a31 * scalar, a32 * scalar, a33 * scalar, a34 * scalar,
				a41 * scalar, a42 * scalar, a43 * scalar, a44 * scalar
			};
		}

		constexpr Matrix4x4& operator+=(const Matrix4x4& other) noexcept
		{
			a11 += other.a11; a12 += other.a12; a13 += other.a13; a14 += other.a14;
			a21 += other.a21; a22 += other.a22; a23 += other.a23; a24 += other.a24;
			a31 += other.a31; a32 += other.a32; a33 += other.a33; a34 += other.a34;
			a41 += other.a41; a42 += other.a42; a43 += other.a43; a44 += other.a44;
			return *this;
		}
		constexpr Matrix4x4& operator-=(const Matrix4x4& other) noexcept
		{
			a11 -= other.a11; a12 -= other.a12; a13 -= other.a13; a14 -= other.a14;
			a21 -= other.a21; a22 -= other.a22; a23 -= other.a23; a24 -= other.a24;
			a31 -= other.a31; a32 -= other.a32; a33 -= other.a33; a34 -= other.a34;
			a41 -= other.a41; a42 -= other.a42; a43 -= other.a43; a44 -= other.a44;
			return *this;
		}
		constexpr Matrix4x4& operator*=(const Matrix4x4& other) noexcept
		{
			return *this = *this * other;
		}
		template <Concept::Number T>
		constexpr Matrix4x4& operator*=(T scalar) noexcept
		{
			a11 *= scalar; a12 *= scalar; a13 *= scalar; a14 *= scalar;
			a21 *= scalar; a22 *= scalar; a23 *= scalar; a24 *= scalar;
			a31 *= scalar; a32 *= scalar; a33 *= scalar; a34 *= scalar;
			a41 *= scalar; a42 *= scalar; a43 *= scalar; a44 *= scalar;
			return *this;
		}

	public:
		constexpr f32 Determinant() const noexcept
		{
			return a11 * MAT3x3_DET(a22, a23, a24, a32, a33, a34, a42, a43, a44)
				- a12 * MAT3x3_DET(a21, a23, a24, a31, a33, a34, a41, a43, a44)
				+ a13 * MAT3x3_DET(a21, a22, a24, a31, a32, a34, a41, a42, a44)
				- a14 * MAT3x3_DET(a21, a22, a23, a31, a32, a33, a41, a42, a43);
		}

		Matrix4x4& Inverse() noexcept
		{
			f32 determinant = Determinant();
			if (determinant == 0.f || !isfinite(1.f / determinant)) return *this;
			determinant = 1.0f / determinant;

			const f32 m11 = a11, m12 = a12, m13 = a13, m14 = a14;
			const f32 m21 = a21, m22 = a22, m23 = a23, m24 = a24;
			const f32 m31 = a31, m32 = a32, m33 = a33, m34 = a34;
			const f32 m41 = a41, m42 = a42, m43 = a43, m44 = a44;

			a11 = determinant * (m22 * (m33 * m44 - m34 * m43) - m23 * (m32 * m44 - m34 * m42) + m24 * (m32 * m43 - m33 * m42));
			a12 = -determinant * (m12 * (m33 * m44 - m34 * m43) - m13 * (m32 * m44 - m34 * m42) + m14 * (m32 * m43 - m33 * m42));
			a13 = determinant * (m12 * (m23 * m44 - m24 * m43) - m13 * (m22 * m44 - m24 * m42) + m14 * (m22 * m43 - m23 * m42));
			a14 = -determinant * (m12 * (m23 * m34 - m24 * m33) - m13 * (m22 * m34 - m24 * m32) + m14 * (m22 * m33 - m23 * m32));

			a21 = -determinant * (m21 * (m33 * m44 - m34 * m43) - m23 * (m31 * m44 - m34 * m41) + m24 * (m31 * m43 - m33 * m41));
			a22 = determinant * (m11 * (m33 * m44 - m34 * m43) - m13 * (m31 * m44 - m34 * m41) + m14 * (m31 * m43 - m33 * m41));
			a23 = -determinant * (m11 * (m23 * m44 - m24 * m43) - m13 * (m21 * m44 - m24 * m41) + m14 * (m21 * m43 - m23 * m41));
			a24 = determinant * (m11 * (m23 * m34 - m24 * m33) - m13 * (m21 * m34 - m24 * m31) + m14 * (m21 * m33 - m23 * m31));

			a31 = determinant * (m21 * (m32 * m44 - m34 * m42) - m22 * (m31 * m44 - m34 * m41) + m24 * (m31 * m42 - m32 * m41));
			a32 = -determinant * (m11 * (m32 * m44 - m34 * m42) - m12 * (m31 * m44 - m34 * m41) + m14 * (m31 * m42 - m32 * m41));
			a33 = determinant * (m11 * (m22 * m44 - m24 * m42) - m12 * (m21 * m44 - m24 * m41) + m14 * (m21 * m42 - m22 * m41));
			a34 = -determinant * (m11 * (m22 * m34 - m24 * m32) - m12 * (m21 * m34 - m24 * m31) + m14 * (m21 * m32 - m22 * m31));

			a41 = -determinant * (m21 * (m32 * m43 - m33 * m42) - m22 * (m31 * m43 - m33 * m41) + m23 * (m31 * m42 - m32 * m41));
			a42 = determinant * (m11 * (m32 * m43 - m33 * m42) - m12 * (m31 * m43 - m33 * m41) + m13 * (m31 * m42 - m32 * m41));
			a43 = -determinant * (m11 * (m22 * m43 - m23 * m42) - m12 * (m21 * m43 - m23 * m41) + m13 * (m21 * m42 - m22 * m41));
			a44 = determinant * (m11 * (m22 * m33 - m23 * m32) - m12 * (m21 * m33 - m23 * m31) + m13 * (m21 * m32 - m22 * m31));

			return *this;
		}
		Matrix4x4 Inversed() const noexcept
		{
			Matrix4x4 result = *this;
			return result.Inverse();
		}

		constexpr Matrix4x4& InverseAffine() noexcept
		{
			a14 = a24 = a34 = 0.0f;
			Swap(a12, a21);
			Swap(a13, a31);
			Swap(a32, a23);

			f32 tx = a41, ty = a42, tz = a43;
			a41 = -(tx * a11 + ty * a21 + tz * a31);
			a42 = -(tx * a12 + ty * a22 + tz * a32);
			a43 = -(tx * a13 + ty * a23 + tz * a33);
			a44 = 1.0f;

			return *this;
		}
		constexpr Matrix4x4 InversedAffine() const noexcept
		{
			Matrix4x4 result = *this;
			return result.InverseAffine();
		}

		constexpr Matrix4x4& Transpose()
		{
			Swap(a12, a21);
			Swap(a13, a31);
			Swap(a14, a41);
			Swap(a23, a32);
			Swap(a24, a42);
			Swap(a34, a43);
			return *this;
		}
		constexpr Matrix4x4 Transposed() const noexcept
		{
			Matrix4x4 result = *this;
			return result.Transpose();
		}

		constexpr Matrix4x4 Adjoint() const noexcept
		{
			Matrix4x4 result;

			result.a11 = MAT3x3_DET(a22, a23, a24, a32, a33, a34, a42, a43, a44);
			result.a12 = -MAT3x3_DET(a12, a13, a14, a32, a33, a34, a42, a43, a44);
			result.a13 = MAT3x3_DET(a12, a13, a14, a22, a23, a24, a42, a43, a44);
			result.a14 = -MAT3x3_DET(a12, a13, a14, a22, a23, a24, a32, a33, a34);

			result.a21 = -MAT3x3_DET(a21, a23, a24, a31, a33, a34, a41, a43, a44);
			result.a22 = MAT3x3_DET(a11, a13, a14, a31, a33, a34, a41, a43, a44);
			result.a23 = -MAT3x3_DET(a11, a13, a14, a21, a23, a24, a41, a43, a44);
			result.a24 = MAT3x3_DET(a11, a13, a14, a21, a23, a24, a31, a33, a34);

			result.a31 = MAT3x3_DET(a21, a22, a24, a31, a32, a34, a41, a42, a44);
			result.a32 = -MAT3x3_DET(a11, a12, a14, a31, a32, a34, a41, a42, a44);
			result.a33 = MAT3x3_DET(a11, a12, a14, a21, a22, a24, a41, a42, a44);
			result.a34 = -MAT3x3_DET(a11, a12, a14, a21, a22, a24, a31, a32, a34);

			result.a41 = -MAT3x3_DET(a21, a22, a23, a31, a32, a33, a41, a42, a43);
			result.a42 = MAT3x3_DET(a11, a12, a13, a31, a32, a33, a41, a42, a43);
			result.a43 = -MAT3x3_DET(a11, a12, a13, a21, a22, a23, a41, a42, a43);
			result.a44 = MAT3x3_DET(a11, a12, a13, a21, a22, a23, a31, a32, a33);

			return result;
		}
		Matrix4x4 Abs() const noexcept
		{
			return {
				fabsf(a11), fabsf(a12), fabsf(a13), fabsf(a14),
				fabsf(a21), fabsf(a22), fabsf(a23), fabsf(a24),
				fabsf(a31), fabsf(a32), fabsf(a33), fabsf(a34),
				fabsf(a41), fabsf(a42), fabsf(a43), fabsf(a44)
			};
		}

		constexpr Vector3<f32> Transform(const Vector3<f32>& axis) const noexcept
		{
			f32 w = a14 * axis.x + a24 * axis.y + a34 * axis.z + a44;
			return {
				(a11 * axis.x + a21 * axis.y + a31 * axis.z + a41) / w,
				(a12 * axis.x + a22 * axis.y + a32 * axis.z + a42) / w,
				(a13 * axis.x + a23 * axis.y + a33 * axis.z + a43) / w
			};
		}
		constexpr Vector3<f32> TransformNormal(const Vector3<f32>& axis) const noexcept
		{
			return {
				(axis.x * a11 + axis.y * a21 + axis.z * a31),
				(axis.x * a12 + axis.y * a22 + axis.z * a32),
				(axis.x * a13 + axis.y * a23 + axis.z * a33)
			};
		}

	public:
		constexpr Vector3<f32> LookVector() const noexcept
		{
			return { a31, a32, a33 };
		}
		constexpr Vector3<f32> UpVector() const noexcept
		{
			return { a21, a22, a23 };
		}
		constexpr Vector3<f32> RightVector() const noexcept
		{
			return { a11, a12, a13 };
		}

		constexpr Vector3<f32> GetTranslation() const noexcept
		{
			return { a41, a42, a43 };
		}
		constexpr Vector3<f32> GetScale() const noexcept
		{
			Vector3 xAxis(a11, a12, a13);
			Vector3 yAxis(a21, a22, a23);
			Vector3 zAxis(a31, a32, a33);
			return { xAxis.Length(), yAxis.Length(), zAxis.Length() };
		}
		Vector3<f32> GetEulerAnglesXYZ() const noexcept
		{
			f32 pitch, yaw, roll;

			if (fabsf(a31) < 0.999999f)
			{
				yaw = asinf(-a31);
				pitch = atan2f(a32, a33);
				roll = atan2f(a21, a11);
			}
			else
			{
				yaw = (a31 < 0.0f ? static_cast<f32>(Constants::Pi) * 0.5f : -static_cast<f32>(Constants::Pi) * 0.5f);
				pitch = atan2f(-a12, a22);
				roll = 0.0f;
			}

			return {
				ToDegrees(pitch),
				ToDegrees(yaw),
				ToDegrees(roll)
			};
		}
		Vector3<f32> GetEulerAnglesYXZ() const noexcept
		{
			f32 pitch, yaw, roll;

			if (fabs(a32) < 0.999999f)
			{
				yaw = atan2f(a13, a33);
				pitch = asinf(-a23);
				roll = atan2f(a21, a22);
			}
			else
			{
				yaw = atan2f(-a31, a11);
				pitch = (a32 < 0.0f ? static_cast<f32>(Constants::Pi) * 0.5f : -static_cast<f32>(Constants::Pi) * 0.5f);
				roll = 0.0f;
			}

			return {
				ToDegrees(pitch),
				ToDegrees(yaw),
				ToDegrees(roll)
			};
		}
		Vector3<f32> GetEulerAnglesZYX() const noexcept
		{
			return Transposed().GetEulerAnglesXYZ();
		}
		Quaternion GetQuaternion() const noexcept
		{
			f32 trace = a11 + a22 + a33;
			Quaternion q;

			if (trace > 0.0f)
			{
				f32 s = 0.5f / sqrtf(trace + 1.0f);
				q.w = 0.25f / s;
				q.x = (a23 - a32) * s;
				q.y = (a31 - a13) * s;
				q.z = (a12 - a21) * s;
			}
			else
			{
				if (a11 > a22 && a11 > a33)
				{
					f32 s = 2.0f * sqrtf(1.0f + a11 - a22 - a33);
					q.w = (a23 - a32) / s;
					q.x = 0.25f * s;
					q.y = (a12 + a21) / s;
					q.z = (a13 + a31) / s;
				}
				else if (a22 > a33)
				{
					f32 s = 2.0f * sqrtf(1.0f + a22 - a11 - a33);
					q.w = (a31 - a13) / s;
					q.x = (a12 + a21) / s;
					q.y = 0.25f * s;
					q.z = (a23 + a32) / s;
				}
				else
				{
					f32 s = 2.0f * sqrtf(1.0f + a33 - a11 - a22);
					q.w = (a12 - a21) / s;
					q.x = (a13 + a31) / s;
					q.y = (a23 + a32) / s;
					q.z = 0.25f * s;
				}
			}
			return q.Normalize();
		}

	public:
		static constexpr Matrix4x4 FromQuaternion(const Quaternion& q) noexcept
		{
			f32 xx = q.x * q.x;
			f32 yy = q.y * q.y;
			f32 zz = q.z * q.z;
			f32 xy = q.x * q.y;
			f32 xz = q.x * q.z;
			f32 yz = q.y * q.z;
			f32 wx = q.w * q.x;
			f32 wy = q.w * q.y;
			f32 wz = q.w * q.z;


			return {
				1.0f - 2.0f * (yy + zz), 2.0f * (xy + wz)       , 2.0f * (xz - wy)       , 0.0f,
				2.0f * (xy - wz)       , 1.0f - 2.0f * (xx + zz), 2.0f * (yz + wx)       , 0.0f,
				2.0f * (xz + wy)       , 2.0f * (yz - wx)       , 1.0f - 2.0f * (xx + yy), 0.0f,
				0.0f                   , 0.0f                   , 0.0f                   , 1.0f
			};
		}
		static constexpr Matrix4x4 FromTranslation(const Vector3<f32>& translation) noexcept
		{
			return {
			    1.0f,			0.0f,			0.0f,			0.f,
			    0.0f,			1.0f,			0.0f,			0.f,
			    0.0f,			0.0f,			1.0f,			0.f,
			    translation.X,	translation.Y,	translation.Z,	1.0f
			};
		}
		static constexpr Matrix4x4 FromScale(const Vector3<f32>& scale) noexcept
		{
			return {
			    scale.X,	0.0f,		0.0f,		0.0f,
			    0.0f,		scale.Y,	0.0f,		0.0f,
			    0.0f,		0.0f,		scale.Z,	0.0f,
			    0.0f,		0.0f,		0.0f,		1.0f
			};
		}
	    static Matrix4x4 FromPerspectiveView(f32 fov, f32 aspect, f32 near, f32 far) noexcept
		{
			f32 scale = 1.f / tanf(fov * 0.5f);
			f32 depth = near - far;

			return {
				scale / aspect, 0	  , 0				  , 0 ,
                0			  , scale, 0				  , 0 ,
				0			  ,	0	  , far / depth		  , -1,
				0			  ,	0     , far * near / depth, 0 ,
			};
		}
	    static constexpr Matrix4x4 FromOrthographicView(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) noexcept
		{
			f32 width = right - left;
			f32 height = top - bottom;
			f32 depth = far - near;

			return {
				2.0f / width           , 0.0f                    , 0.0f                 , 0.0f,
				0.0f                   , 2.0f / height           , 0.0f					, 0.0f,
				0.0f                   , 0.0f                    , 1.0f / depth         , 0.0f,
				-(right + left) / width, -(top + bottom) / height, -near / depth        , 1.0f
			};
		}
	    static Matrix4x4 LookAt(const Vector3<f32>& eye, const Vector3<f32>& target, const Vector3<f32>& up) noexcept
        {
			Vector3<f32> zAxis = (target - eye).Normalized();
			Vector3<f32> xAxis = up.Cross(zAxis).Normalized();
			Vector3<f32> yAxis = zAxis.Cross(xAxis);

			return {
				xAxis.X, yAxis.X, zAxis.X, 0,
				xAxis.Y, yAxis.Y, zAxis.Y, 0,
				xAxis.Z, yAxis.Z, zAxis.Z, 0,
				-xAxis.Dot(eye), -yAxis.Dot(eye), -zAxis.Dot(eye), 1
			};
		}
	};
}

constexpr MC::Math::Matrix4x4 operator*(f32 scalar, const MC::Math::Matrix4x4& matrix) noexcept
{
	return matrix * scalar;
}

constexpr MC::Math::Vector4<f32> operator*(const MC::Math::Vector4<f32>& vector, const MC::Math::Matrix4x4& matrix) noexcept
{
	return {
		vector.x * matrix.a11 + vector.y * matrix.a21 + vector.z * matrix.a31 + vector.w * matrix.a41,
		vector.x * matrix.a12 + vector.y * matrix.a22 + vector.z * matrix.a32 + vector.w * matrix.a42,
		vector.x * matrix.a13 + vector.y * matrix.a23 + vector.z * matrix.a33 + vector.w * matrix.a43,
		vector.x * matrix.a14 + vector.y * matrix.a24 + vector.z * matrix.a34 + vector.w * matrix.a44
	};
}

MC_FMT_FORMATTER(MC::Math::Matrix4x4, "Row 0: %f, %f, %f, %f\n"
												 "Row 1: %f, %f, %f, %f\n"
												 "Row 2: %f, %f, %f, %f\n"
												 "Row 3: %f, %f, %f, %f\n",
	MC_FMT_FORMATTER_MEMBER(a11), MC_FMT_FORMATTER_MEMBER(a12), MC_FMT_FORMATTER_MEMBER(a13), MC_FMT_FORMATTER_MEMBER(a14),
	MC_FMT_FORMATTER_MEMBER(a21), MC_FMT_FORMATTER_MEMBER(a22), MC_FMT_FORMATTER_MEMBER(a23), MC_FMT_FORMATTER_MEMBER(a24),
	MC_FMT_FORMATTER_MEMBER(a31), MC_FMT_FORMATTER_MEMBER(a32), MC_FMT_FORMATTER_MEMBER(a33), MC_FMT_FORMATTER_MEMBER(a34),
	MC_FMT_FORMATTER_MEMBER(a41), MC_FMT_FORMATTER_MEMBER(a42), MC_FMT_FORMATTER_MEMBER(a43), MC_FMT_FORMATTER_MEMBER(a44));