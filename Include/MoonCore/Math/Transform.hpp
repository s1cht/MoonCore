#pragma once
#include "MoonCore/Math/Math.hpp"

namespace MC::Math
{
    class Transform
    {
    public:
        explicit constexpr Transform() noexcept
            : m_Position(Vector3D::ZeroVector),
              m_Rotation(Quaternion::Identity()),
              m_Scale(Vector3D(1.f)),
              m_Dirty(true)
        {}
        constexpr Transform(const Vector3D& position) noexcept
            : m_Position(position),
              m_Rotation(Quaternion::Identity()),
              m_Scale(Vector3D(1.f)),
              m_Dirty(true)
        {}
        constexpr Transform(const Vector3D& position, const Vector3D& scale) noexcept
            : m_Position(position),
              m_Rotation(Quaternion::Identity()),
              m_Scale(scale),
              m_Dirty(true)
        {}
        constexpr Transform(const Vector3D& position, const Vector3D& scale, const Quaternion& rotation) noexcept
            : m_Position(position),
              m_Rotation(rotation),
              m_Scale(scale),
              m_Dirty(true)
        {}
        Transform(const Vector3D& position, const Vector3D& scale, const Vector3D& rotation) noexcept
            : m_Position(position),
              m_Rotation(Quaternion::FromEulerAnglesXYZ(rotation)),
              m_Scale(scale),
              m_Dirty(true)
        {}

        Transform(const Transform& other) noexcept
            : m_Position(other.m_Position),
              m_Rotation(other.m_Rotation),
              m_Scale(other.m_Scale),
              m_CachedMatrix(other.m_CachedMatrix),
              m_Dirty(other.m_Dirty.load())
        {}
        Transform(Transform&& other) noexcept
            : m_Position(other.m_Position),
              m_Rotation(other.m_Rotation),
              m_Scale(other.m_Scale),
              m_CachedMatrix(other.m_CachedMatrix),
              m_Dirty(other.m_Dirty.load())
        {}
        Transform& operator=(const Transform& other) noexcept
        {
            m_Position = other.m_Position;
            m_Rotation = other.m_Rotation;
            m_Scale = other.m_Scale;
            m_CachedMatrix = other.m_CachedMatrix;
            m_Dirty.store(other.m_Dirty.load());
            return *this;
        }
        Transform& operator=(Transform&& other) noexcept
        {
            m_Position = Move(other.m_Position);
            m_Rotation = Move(other.m_Rotation);
            m_Scale = Move(other.m_Scale);
            m_CachedMatrix = Move(other.m_CachedMatrix);
            m_Dirty.store(other.m_Dirty.load());
            return *this;
        }

    public:
        Transform operator*(const Transform& other) const noexcept
        {
            Transform result;
            result.m_Rotation = (m_Rotation * other.m_Rotation).Normalized();
            result.m_Scale = m_Scale * other.m_Scale;
            result.m_Position = m_Position + (m_Rotation.RotateVector(m_Scale * other.m_Position));
            result.m_Dirty.store(true);
            return result;
        }
        Vector3D operator*(const Vector3D& v) const noexcept
        {
            return m_Rotation.RotateVector(v * m_Scale) + m_Position;
        }

    public:
        constexpr const Vector3D& Position() const noexcept { return m_Position; }
        constexpr const Quaternion& QRotation() const noexcept { return m_Rotation; }
        Vector3D Rotation() const noexcept { return m_Rotation.ToEulerAnglesXYZ(); }
        constexpr const Vector3D& Scale() const noexcept { return m_Scale; }

        void SetPosition(const Vector3D& position) noexcept
        {
            m_Position = position;
            m_Dirty.store(true);
        }
        void SetRotation(const Quaternion& rotation) noexcept
        {
            m_Rotation = rotation;
            m_Dirty.store(true);
        }
        void SetRotation(const Vector3D& euler) noexcept
        {
            m_Rotation = Quaternion::FromEulerAnglesXYZ(euler);
            m_Dirty.store(true);
        }
        void SetScale(const Vector3D& scale) noexcept
        {
            m_Scale = scale;
            m_Dirty.store(true);
        }

    public:
        const Matrix4x4& Matrix() const noexcept
        {
            if (m_Dirty.load()) RecalculateMatrix();
            return m_CachedMatrix;
        }

        Vector3D LookVector() const noexcept
        {
            if (m_Dirty.load()) RecalculateMatrix();
            return Matrix().LookVector();
        }
        Vector3D UpVector() const noexcept
        {
            if (m_Dirty.load()) RecalculateMatrix();
            return Matrix().UpVector();
        }
        Vector3D RightVector() const noexcept
        {
            if (m_Dirty.load()) RecalculateMatrix();
            return Matrix().RightVector();
        }

        Transform Inverse() const noexcept
        {
            Transform result;
            result.m_Scale = Vector3D(1.f) / m_Scale;
            result.m_Rotation = m_Rotation.Conjugated();
            result.m_Position = result.m_Rotation.RotateVector(-m_Position) * result.m_Scale;
            result.m_Dirty = true;
            return result;
        }

    public:
        static Transform FromMatrix(const Matrix4x4& m) noexcept
        {
            Transform result;
            result.m_Position = m.GetTranslation();
            result.m_Rotation = m.GetQuaternion().Normalized();
            result.m_Scale = m.GetScale();
            result.m_Dirty.store(true);
            return result;
        }
        static Transform FromQuaternion(const Quaternion& rot) noexcept
        {
            Transform result;
            result.m_Position = Vector3D(0.f);
            result.m_Rotation = rot.Normalized();
            result.m_Scale = Vector3D(1.f);
            result.m_Dirty = true;
            return result;
        }
        static Transform LookAt(const Vector3D& pos, const Vector3D& target, const Vector3D& up) noexcept
        {
            Transform result;
            result.m_Position = pos;
            result.m_Rotation = Quaternion::LookAt(pos, target).Normalized();
            result.m_Scale = Vector3D(1.f);
            result.m_Dirty = true;
            return result;
        }

    private:
        void RecalculateMatrix() const noexcept
        {
            m_CachedMatrix =
                Matrix4x4::FromTranslation(m_Position) *
                Matrix4x4::FromQuaternion(m_Rotation) *
                Matrix4x4::FromScale(m_Scale);
            m_Dirty = false;
        }

    private:
        Vector3D m_Position;
        Quaternion m_Rotation;
        Vector3D m_Scale;

        mutable Matrix4x4 m_CachedMatrix;
        mutable Async::AtomicBool m_Dirty;
    };
}
