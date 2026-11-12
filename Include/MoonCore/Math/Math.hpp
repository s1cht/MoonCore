#pragma once
#include "MoonCore/Math/Vector4.hpp"
#include "Matrix4x4.hpp"

namespace MC::Math
{
	struct Frustum
	{
		Vector4D Planes[6];
	
		Frustum() : Planes{ Vector4D::ZeroVector, Vector4D::ZeroVector, Vector4D::ZeroVector, 
		                    Vector4D::ZeroVector, Vector4D::ZeroVector, Vector4D::ZeroVector } 
	    {}
	};

    inline Frustum ExtractFrustumFromMatrix(const Matrix4x4& m) noexcept
    {
        auto col = [&](int c) { return Vector4D(m.a[0][c], m.a[1][c], m.a[2][c], m.a[3][c]); };
        const Vector4D c0 = col(0), c1 = col(1), c2 = col(2), c3 = col(3);

        Frustum f;
        f.Planes[0] = c3 + c0;
        f.Planes[1] = c3 - c0;
        f.Planes[2] = c3 + c1;
        f.Planes[3] = c3 - c1;
        f.Planes[4] = c2;
        f.Planes[5] = c3 - c2;
        for (auto& p : f.Planes)
            p /= Vector3D(p.x, p.y, p.z).Length();
        return f;
    }

    inline i64 Random() noexcept { return rand(); }
    inline i64 Random(i64 min, i64 max) noexcept { return Random() % (max - min + 1) + min; }
    inline f32 RandomFloat() noexcept { return static_cast<f32>(Random()) / static_cast<f32>(RAND_MAX); }
}