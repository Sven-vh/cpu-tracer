#pragma once

#pragma warning(disable: 4201)

namespace Tmpl8 {
	// 196 bytes
	class Ray {
	public:
		Ray() = default;
		Ray(const float3 origin, const float3 direction, const float rayLength = 1e34f, const int rgb = 0)
			: O(origin), D(direction), t(rayLength), voxel(rgb) {
			// calculate reciprocal ray direction for triangles and AABBs
			// TODO: prevent NaNs - or don't
			rD = float3(1 / D.x, 1 / D.y, 1 / D.z);
#if 1
			uint xsign = *(uint*)&D.x >> 31;
			uint ysign = *(uint*)&D.y >> 31;
			uint zsign = *(uint*)&D.z >> 31;
			Dsign = (float3((float)xsign * 2 - 1, (float)ysign * 2 - 1, (float)zsign * 2 - 1) + 1) * 0.5f;
#else
			Dsign = (float3(-copysign(1.0f, D.x), -copysign(1.0f, D.y), -copysign(1.0f, D.z)) + 1) * 0.5f;
#endif
		}

		float3 IntersectionPoint() const;
		float3 LocalIntersectionPoint() const;
		float3 GetNormal() const;
		float3 GetAlbedo() const;
		float2 GetUV() const;
		Material GetMaterial() const;
		int GetMaterialIndex() const;
		// ray data

		//union { struct { float3 O; float dummy1; }; __m128 O4; }; // ray origin,
		//union { struct { float3 D; float dummy2; }; __m128 D4; }; // ray direction, 
		//union { struct { float3 rD; float dummy3; }; __m128 rD4; }; // reciprocal ray direction,


		float3 O;					// ray origin, 12 bytes
		float3 rD;					// reciprocal ray direction, 12 bytes
		float3 D = float3(0);		// ray direction, 12 bytes

		float t = 1e34f;			// ray length, 4 bytes
		float3 Dsign = float3(1);	// inverted ray direction signs, -1 or 1, 12 bytes
		uint voxel = 0;				// 32-bit ARGB color of a voxelhit object index; 0 = NONE, 4 bytes
		int steps = 0;				// number of steps taken in the ray, 4 bytes
		int index = -1;				// index of the voxel, 4 bytes
		int worldIndex = -1;			// index of the world, 4 bytes
		mat4 worldTransform;		// transform of the world, 64 bytes
		mat4 invWorldTransform;		// inverse transform of the world, 64 bytes
#if SPHERES
		float3 N;					// normal at intersection point, 12 bytes
#endif
	private:
		// min3 is used in normal reconstruction.
		__inline static float3 min3(const float3& a, const float3& b) {
			return float3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
		}
	};

	class Cube {
	public:
		Cube() = default;
		Cube(const float3 pos, const float3 size);
		float Intersect(const Ray& ray) const;
#if USE_SIMD
		float IntersectSIMD(const Ray& ray) const;
		union { struct { float3 bmin; float dummy1; }; __m128 bmin4; };
		union { struct { float3 bmax; float dummy2; }; __m128 bmax4; };
#endif
		bool Contains(const float3& pos) const;
		float3 b[2];

		// min and max union for SIMD

		float3 GetSize() const;
	};
	struct DDAState {
		int3 stepDirection;				// 16 bytes
		uint posX, posY, posZ;			// 12 bytes
		float travelDistance;				// 4 bytes
		float3 deltaDistance; 			// 12 bytes
		float dummy1 = 0;		// 4 bytes
		float3 nextIntersection;		// 12 bytes
		float dummy2 = 0;		// 4 bytes, 64 bytes total
	};

}