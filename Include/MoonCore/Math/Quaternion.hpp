#pragma once
#include "MathCore.hpp"
#include "MoonCore/Utility/Format.hpp"

namespace MC::Math
{
	struct Quaternion
	{
	public:
		union
		{
			struct { f32 w, x, y, z; };
			struct { f32 W, X, Y, Z; };
		};
	public:
	    constexpr Quaternion() noexcept
			: w(1.0f), x(0.0f), y(0.0f), z(0.0f)
		{}
	    Quaternion(f32 scalar, const Vector3<f32>& axis, bool degrees = false)
			: w(1.0f), x(0.0f), y(0.0f), z(0.0f)
	    {
			if (degrees) scalar = ToRadians(scalar);
			if (!isfinite(scalar) || !axis.LengthSquared()) return;

			f32 halfAngle = scalar * 0.5f;
			f32 sinHalf = sinf(halfAngle);
			Vector3 normAxis = axis.Normalized();

			w = cosf(halfAngle);
			x = sinHalf * normAxis.x;
			y = sinHalf * normAxis.y;
			z = sinHalf * normAxis.z;

			Normalize();
		}

	    constexpr Quaternion(const Vector4<f32>& val) noexcept
			: w(val.w), x(val.x), y(val.y), z(val.z)
		{}

		constexpr Quaternion(f32 wValue, f32 xValue, f32 yValue, f32 zValue) noexcept
			: w(wValue), x(xValue), y(yValue), z(zValue)
		{}

		constexpr Quaternion(const Quaternion&) noexcept = default;
		constexpr Quaternion(Quaternion&&) noexcept = default;
		constexpr Quaternion& operator=(const Quaternion&) noexcept = default;
		constexpr Quaternion& operator=(Quaternion&&) noexcept = default;

	    constexpr ~Quaternion() = default;

	public:
		static constexpr Quaternion Identity() noexcept
		{
			return Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
		}

	public:
	    bool operator==(const Quaternion& other) const noexcept
		{
			return fabs(w - other.w) < Epsilon<f32> &&
				fabs(x - other.x) < Epsilon<f32> &&
				fabs(y - other.y) < Epsilon<f32> &&
				fabs(z - other.z) < Epsilon<f32>;
		}

		constexpr Quaternion operator*(f32 number) const noexcept
		{
			return { w * number, x * number, y * number, z * number };
		}
	    constexpr Quaternion operator*(const Quaternion& other) const noexcept
		{
			return {
				w * other.w - x * other.x - y * other.y - z * other.z,
				w * other.x + x * other.w + y * other.z - z * other.y,
				w * other.y - x * other.z + y * other.w + z * other.x,
				w * other.z + x * other.y - y * other.x + z * other.w
			};
		}

		constexpr Quaternion& operator*=(f32 number) noexcept
		{
			return *this = *this * number;
		}
		constexpr Quaternion& operator*=(const Quaternion& other) noexcept
		{
			return *this = *this * other;
		}

	    Quaternion operator/(f32 number) const noexcept
		{
			if (fabs(number) <= Epsilon<f32>) return Identity();

			Quaternion result;
			result.x = x / number;
			result.y = y / number;
			result.z = z / number;
			result.w = w / number;
			return result;
		}
	    Quaternion operator/(const Quaternion& other) const noexcept
		{
			return *this * other.Inverted();
		}
		Quaternion& operator/=(f32 number) noexcept
		{
			return *this = *this / number;
		}
		Quaternion& operator/=(const Quaternion& other) noexcept
		{
			return *this = *this * other.Inverted();
		}

		constexpr Quaternion operator+(const Quaternion& other) const noexcept
		{
			Quaternion result;
			result.x = x + other.x;
			result.y = y + other.y;
			result.z = z + other.z;
			result.w = w + other.w;
			return result;
		}
		constexpr Quaternion& operator+=(const Quaternion& other) noexcept
		{
			x += other.x;
			y += other.y;
			z += other.z;
			w += other.w;
			return *this;
		}

		constexpr Quaternion operator-(const Quaternion& other) const noexcept
		{
			Quaternion result;
			result.x = x - other.x;
			result.y = y - other.y;
			result.z = z - other.z;
			result.w = w - other.w;
			return result;
		}
		constexpr Quaternion& operator-=(const Quaternion& other) noexcept
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			w -= other.w;
			return *this;
		}

	public:
		f32 Length() const noexcept
		{
			return sqrtf(SQUARE_SUM_4(w, x, y, z));
		}
		constexpr f32 LengthSquared() const noexcept
		{
			return SQUARE_SUM_4(w, x, y, z);
		}

		constexpr f32 Dot(const Quaternion& other) const noexcept
		{
			return x * other.x + y * other.y + z * other.z + w * other.w;
		}

	    Quaternion& Normalize() noexcept
		{
			f32 length = LengthSquared();
			if (length > Epsilon<f32>)
			{
				f32 lengthInverse = 1.0f / sqrtf(length);
				w *= lengthInverse;
				x *= lengthInverse;
				y *= lengthInverse;
				z *= lengthInverse;
			}
			return *this;
		}
	    Quaternion Normalized() const noexcept
		{
			f32 length = LengthSquared();
			if (length > Epsilon<f32>)
			{
				f32 lengthInverse = 1.0f / sqrtf(length);
				return { w * lengthInverse, x * lengthInverse, y * lengthInverse, z * lengthInverse };
			}
			return Identity();
		}

		constexpr Quaternion& Conjugate() noexcept
		{
			x = -x;
			y = -y;
			z = -z;
			return *this;
		}
		constexpr Quaternion Conjugated() const noexcept
		{
			return { w, -x, -y, -z };
		}

		Quaternion& Inverse() noexcept
		{
			f32 lengthSquared = LengthSquared();
			if (fabsf(lengthSquared - 1.f) < Epsilon<f32>) return Conjugate();
			f32 invertedLength = 1.0f / lengthSquared;
			Conjugate();
			return *this *= invertedLength;
		}
		Quaternion Inverted() const noexcept
		{
			f32 lengthSquared = LengthSquared();
			if (fabsf(lengthSquared - 1.f) < Epsilon<f32>) return Conjugated();
			f32 invertedLength = 1.0f / lengthSquared;
			return Conjugated() * invertedLength;
		}

	    Quaternion Slerp(Quaternion other, f32 t) const noexcept
		{
			f32 d = Dot(other);
			if (d < 0.f) { other = other * -1.f; d = -d; }                        
			if (d > 0.9995f) return (*this * (1.f - t) + other * t).Normalized();
			const f32 angle = acosf(d), s = sinf(angle);
			return *this * (sinf((1.f - t) * angle) / s) + other * (sinf(t * angle) / s);
		}

	    Vector3<f32> RotateVector(const Vector3<f32>& vec) const noexcept
		{
			Vector3 u(x, y, z);
			f32 uvD = u.Dot(vec);
			Vector3 uvC = u.Cross(vec);
			return  (u * uvD + uvC * w) * 2.f + vec * (w * w - u.LengthSquared());
		}

	public:
	    static Quaternion LookAt(const Vector3<f32>& position, const Vector3<f32>& lookAtPosition) noexcept
	    {
			Vector3<f32> forwardVector = (lookAtPosition - position).Normalized();
			Vector3<f32> referenceForward = Vector3<f32>::ForwardVector;

			f32 dot = referenceForward.Dot(forwardVector);

			dot = Clamp(dot, -1.0f, 1.0f);

			if (fabsf(dot + 1.0f) < Epsilon<f32>)
			{
				Vector3<f32> axis = Vector3<f32>::UpVector;
				if (fabsf(referenceForward.Dot(axis)) > 0.999f)
					axis = Vector3<f32>::RightVector;

				return FromAxisAngle(axis, static_cast<f32>(Constants::Pi));
			}

			if (fabsf(dot - 1.0f) < 1e-6f) return Identity();
			f32 angle = acosf(dot);
			Vector3<f32> axis = referenceForward.Cross(forwardVector).Normalized();
			return FromAxisAngle(axis, angle);
	    }

		static Quaternion FromEulerAnglesXYZ(f32 ax, f32 ay, f32 az) noexcept
		{
			const f32 cx = cosf(ax * .5f), sx = sinf(ax * .5f);
			const f32 cy = cosf(ay * .5f), sy = sinf(ay * .5f);
			const f32 cz = cosf(az * .5f), sz = sinf(az * .5f);
			return { 
			    cx * cy * cz - sx * sy * sz, 
			    sx * cy * cz + cx * sy * sz, 
			    cx * sy * cz - sx * cy * sz, 
			    cx * cy * sz + sx * sy * cz 
			};
		}

		static Quaternion FromEulerAnglesXYZ(Vector3D rotation) noexcept
		{
			return FromEulerAnglesXYZ(rotation.x, rotation.y, rotation.z);
		}

		Vector3<f32> ToEulerAnglesXYZ() const noexcept
		{
			const f32 ay = asinf(Clamp(2.f * (x * z + w * y), -1.f, 1.f));
			const f32 ax = atan2f(2.f * (w * x - y * z), 1.f - 2.f * (x * x + y * y));
			const f32 az = atan2f(2.f * (w * z - x * y), 1.f - 2.f * (y * y + z * z));
			return { ax, ay, az };
		}

		static Quaternion FromEulerAnglesYXZ(f32 yaw, f32 pitch, f32 roll) noexcept
		{
			f32 cx = cosf(pitch * 0.5f); f32 sx = sinf(pitch * 0.5f);
			f32 cy = cosf(yaw * 0.5f); f32 sy = sinf(yaw * 0.5f);
			f32 cz = cosf(roll * 0.5f); f32 sz = sinf(roll * 0.5f);

			return {
				cy * cx * cz + sy * sx * sz,
				cy * sx * cz + sy * cx * sz,
				sy * cx * cz - cy * sx * sz,
				cy * cx * sz - sy * sx * cz
			};
		}

		static Quaternion FromEulerAnglesYXZ(const Vector3<f32>& angles) noexcept
	    {
			return FromEulerAnglesYXZ(angles.x, angles.y, angles.z);
	    }

		Vector3<f32> ToEulerAnglesYXZ() const noexcept
		{
			const f32 ax = asinf(Clamp(2.f * (w * x - y * z), -1.f, 1.f));
			const f32 ay = atan2f(2.f * (x * z + w * y), 1.f - 2.f * (x * x + y * y));
			const f32 az = atan2f(2.f * (x * y + w * z), 1.f - 2.f * (x * x + z * z));
			return { ax, ay, az };
		}
		static Quaternion FromEulerAnglesZYX(const Vector3<f32>& angles) noexcept
	    {
			return FromEulerAnglesZYX(angles.x, angles.y, angles.z);
	    }
		static Quaternion FromEulerAnglesZYX(f32 ax, f32 ay, f32 az) noexcept
	    {
			const f32 cx = cosf(ax * .5f), sx = sinf(ax * .5f);
			const f32 cy = cosf(ay * .5f), sy = sinf(ay * .5f);
			const f32 cz = cosf(az * .5f), sz = sinf(az * .5f);
			return { 
			    cz * cy * cx + sz * sy * sx, 
			    cz * cy * sx - sz * sy * cx, 
			    cz * sy * cx + sz * cy * sx, 
			    sz * cy * cx - cz * sy * sx 
			};
	    }

		Vector3<f32> ToEulerAnglesZYX() const noexcept
		{
			const f32 ay = asinf(Clamp(2.f * (w * y - z * x), -1.f, 1.f));
			const f32 az = atan2f(2.f * (w * z + x * y), 1.f - 2.f * (y * y + z * z));
			const f32 ax = atan2f(2.f * (w * x + y * z), 1.f - 2.f * (x * x + y * y));
			return { ax, ay, az };
		}

		static Quaternion FromAxisAngle(const Vector3<f32>& axis, f32 angle) noexcept
	    {
			f32 halfAngle = angle * 0.5f;
			f32 sinA = sinf(halfAngle);
			return {
				cosf(halfAngle),
				axis.X * sinA,
				axis.Y * sinA,
				axis.Z * sinA
			};
	    }
	};
}

constexpr MC::Math::Quaternion operator*(f32 number, const MC::Math::Quaternion& other) noexcept
{
    return other * number;
}

MC_FMT_FORMATTER(MC::Math::Quaternion, "%f, %f, %f, %f",
	MC_FMT_FORMATTER_MEMBER(w), MC_FMT_FORMATTER_MEMBER(x), MC_FMT_FORMATTER_MEMBER(y),
	MC_FMT_FORMATTER_MEMBER(z));