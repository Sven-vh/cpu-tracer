#include "precomp.h"

float3 Tmpl8::Ray::IntersectionPoint() const {
	float3 hit = O + t * D;
	return hit;
}

float3 Tmpl8::Ray::LocalIntersectionPoint() const {
	float3 hit = O + t * D;
	hit = invWorldTransform.TransformPoint(hit);
	return hit;
}

float3 Ray::GetNormal() const {
#if SPHERES
	//return the normal of the sphere at the nearest intersection
	return N;
#else
	float3 intersectionPoint = O + t * D;
	// Transform the intersection point into object space
	intersectionPoint = invWorldTransform.TransformPoint(intersectionPoint);

	// Calculate the normal in object space with the original method
	const float3 I1 = intersectionPoint * WORLDSIZE;
	const float3 fG = fracf(I1);
	const float3 d = min3(fG, 1.0f - fG);
	const float mind = min(min(d.x, d.y), d.z);
	const float3 sign = Dsign * 2 - 1;
	float3 normal = float3(mind == d.x ? sign.x : 0, mind == d.y ? sign.y : 0, mind == d.z ? sign.z : 0);

	// Transform the normal using the original sub-object transform without translation (for normals, w = 0)
	normal = worldTransform.TransformVector(normal);

	// Normalize the transformed normal to address potential scaling/shearing issues
	return normalize(normal);
#endif
}


float3 Ray::GetAlbedo() const {
	float3 textureColor = float3(1, 1, 1);
	Material m = GetMaterial();
	if (m.hasTexture) {
		textureColor =
			m.GetTextureColor(GetUV());
	}

	float3 voxelColor;
	uint color = voxel;
	float r = ((color >> 16) & 0xFF) / 255.0f; // Extract red
	float g = ((color >> 8) & 0xFF) / 255.0f;  // Extract green
	float b = (color & 0xFF) / 255.0f;         // Extract blue
	voxelColor = float3(r, g, b);

	if (m.hasTexture && m.combineTexture) {
		return textureColor * voxelColor;
	} else if (m.hasTexture) {
		return textureColor;
	} else {
		return float3(r, g, b);
	}
}

float2 Ray::GetUV() const {
	// Calculate the normal at the intersection point
	float3 N = GetNormal();
	float3 hitPos = O + t * D;
	hitPos = hitPos * WORLDSIZE;

	// Calculate UV coordinates based on the face normal
	float u, v;
	// Calculate UV coordinates based on the face normal without SIMD
	if (fabs(N.x) > fabs(N.y) && fabs(N.x) > fabs(N.z)) {
		u = fmod(hitPos.z, 1.0f);
		v = fmod(hitPos.y, 1.0f);
	} else if (fabs(N.y) > fabs(N.x) && fabs(N.y) > fabs(N.z)) {
		u = fmod(hitPos.x, 1.0f);
		v = fmod(hitPos.z, 1.0f);
	} else {
		u = fmod(hitPos.x, 1.0f);
		v = fmod(hitPos.y, 1.0f);
	}

	if (N.x < 0 || N.y < 0 || N.z < 0) {
		u = 1.0f - u;
		v = 1.0f - v;
	}

	v = 1.0f - v;

	u = clamp(u, 0.0f, 1.0f);
	v = clamp(v, 0.0f, 1.0f);

	return float2(u, v);
}

Material Tmpl8::Ray::GetMaterial() const {
	//get material index form the first 8 bits of the voxel
	int newIndex = voxel >> 24;
	return MaterialList[newIndex];
}

int Tmpl8::Ray::GetMaterialIndex() const {
	//get material index form the first 8 bits of the voxel
	return voxel >> 24;
}

Cube::Cube(const float3 pos, const float3 size) {
	// set cube bounds
	b[0] = pos;
	b[1] = pos + size;

#if SIMD
	bmin = b[0];
	bmax = b[1];
#endif
}

float Cube::Intersect(const Ray& ray) const {
	// test if the ray intersects the cube
	const int signx = ray.D.x < 0, signy = ray.D.y < 0, signz = ray.D.z < 0;
	float tmin = (b[signx].x - ray.O.x) * ray.rD.x;
	float tmax = (b[1 - signx].x - ray.O.x) * ray.rD.x;
	const float tymin = (b[signy].y - ray.O.y) * ray.rD.y;
	const float tymax = (b[1 - signy].y - ray.O.y) * ray.rD.y;
	if (tmin > tymax || tymin > tmax) goto miss;
	tmin = max(tmin, tymin), tmax = min(tmax, tymax);
	const float tzmin = (b[signz].z - ray.O.z) * ray.rD.z;
	const float tzmax = (b[1 - signz].z - ray.O.z) * ray.rD.z;
	if (tmin > tzmax || tzmin > tmax) goto miss; // yeah c has 'goto' ;)
	if ((tmin = max(tmin, tzmin)) > 0) return tmin;
miss:
	return 1e34f;
}


//Source: https://jacco.ompf2.com/2022/04/18/how-to-build-a-bvh-part-2-faster-rays/
#if USE_SIMD
float Tmpl8::Cube::IntersectSIMD(const Ray& ray) const {
	static __m128 mask4 = _mm_cmpeq_ps(_mm_setzero_ps(), _mm_set_ps(1, 0, 0, 0));
	__m128 t1 = _mm_mul_ps(_mm_sub_ps(_mm_and_ps(bmin4, mask4), ray.O4), ray.rD4);
	__m128 t2 = _mm_mul_ps(_mm_sub_ps(_mm_and_ps(bmax4, mask4), ray.O4), ray.rD4);
	__m128 vmax4 = _mm_max_ps(t1, t2), vmin4 = _mm_min_ps(t1, t2);
	float tmax = min(vmax4.m128_f32[0], min(vmax4.m128_f32[1], vmax4.m128_f32[2]));
	float tmin = max(vmin4.m128_f32[0], max(vmin4.m128_f32[1], vmin4.m128_f32[2]));
	if (tmax >= tmin && tmin < ray.t && tmax > 0) return tmin; else return 1e34f;
}
#endif
bool Cube::Contains(const float3& pos) const {
	// test if pos is inside the cube
	return pos.x >= b[0].x && pos.y >= b[0].y && pos.z >= b[0].z &&
		pos.x <= b[1].x && pos.y <= b[1].y && pos.z <= b[1].z;
}

float3 Tmpl8::Cube::GetSize() const {
	return b[1] - b[0];
}