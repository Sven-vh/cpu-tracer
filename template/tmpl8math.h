// Template, IGAD version 3
// Get the latest version from: https://github.com/jbikker/tmpl8
// IGAD/NHTV/BUAS/UU - Jacco Bikker - 2006-2023

// In this file: a basic, but quite complete set of math functionality.
// Overview:
// Line 27 - 120: Basic vector type definition. This covers all variants that
//     are normally encountered during game / graphics development.
// Line 128 - 540: Operations on the basic types. Based on NVIDIA's CUDA math
//     library. Pretty complete but feel free to add your own operations.
// Line 542 - 784: Matrix classes (2x2, 4x4) and operations on them.
// Line 786 - 905: Quaternion class.
// Line 907 - 961: Axis-aligned bounding box (AABB) class.

// These math classes have been battle-tested in the Lighthouse 2 real-time
// path tracing framework, as well as numerous other math-heavy projects, see
// https://github.com/jbikker for examples.

#pragma once

namespace Tmpl8 {

#pragma warning ( push )
#pragma warning ( disable: 4201 /* nameless struct / union */ )

	// vector type placeholders, carefully matching OpenCL's layout and alignment
	struct ALIGN(8) int2 {
		int2() = default;
		int2(const int a, const int b) : x(a), y(b) {}
		int2(const int a) : x(a), y(a) {}
		union { struct { int x, y; }; int cell[2]; };
		int& operator [] (const int n) { return cell[n]; }
	};
	struct ALIGN(8) uint2 {
		uint2() = default;
		uint2(const int a, const int b) : x(a), y(b) {}
		uint2(const uint a) : x(a), y(a) {}
		union { struct { uint x, y; }; uint cell[2]; };
		uint& operator [] (const int n) { return cell[n]; }
	};
	struct ALIGN(8) float2 {
		float2() = default;
		float2(const float a, const float b) : x(a), y(b) {}
		float2(const float a) : x(a), y(a) {}
		float2(const int2 a) : x((float)a.x), y((float)a.y) {}
		union { struct { float x, y; }; float cell[2]; };
		float& operator [] (const int n) { return cell[n]; }
	};
	struct int3;
	struct ALIGN(16) int4 {
		int4() = default;
		int4(const int a, const int b, const int c, const int d) : x(a), y(b), z(c), w(d) {}
		int4(const int a) : x(a), y(a), z(a), w(a) {}
		int4(const int3 & a, const int d);
		union { struct { int x, y, z, w; }; int cell[4]; };
		int& operator [] (const int n) { return cell[n]; }
	};
	struct ALIGN(16) int3 {
		int3() = default;
		int3(const int a, const int b, const int c) : x(a), y(b), z(c) {}
		int3(const int a) : x(a), y(a), z(a) {}
		int3(const int4 a) : x(a.x), y(a.y), z(a.z) {}
		union { struct { int x, y, z; int dummy; }; int cell[4]; };
		int& operator [] (const int n) { return cell[n]; }
	};
	struct uint3;
	struct ALIGN(16) uint4 {
		uint4() = default;
		uint4(const uint a, const uint b, const uint c, const uint d) : x(a), y(b), z(c), w(d) {}
		uint4(const uint a) : x(a), y(a), z(a), w(a) {}
		uint4(const uint3 & a, const uint d);
		union { struct { uint x, y, z, w; }; uint cell[4]; };
		uint& operator [] (const int n) { return cell[n]; }
	};
	struct ALIGN(16) uint3 {
		uint3() = default;
		uint3(const uint a, const uint b, const uint c) : x(a), y(b), z(c) {}
		uint3(const uint a) : x(a), y(a), z(a) {}
		uint3(const uint4 a) : x(a.x), y(a.y), z(a.z) {}
		union { struct { uint x, y, z; uint dummy; }; uint cell[4]; };
		uint& operator [] (const int n) { return cell[n]; }
	};
	struct float3;
	struct ALIGN(16) float4 {
		float4() = default;
		float4(const float a, const float b, const float c, const float d) : x(a), y(b), z(c), w(d) {}
		float4(const float a) : x(a), y(a), z(a), w(a) {}
		float4(const float3 & a, const float d);
		float4(const float3 & a);
		float4(const uint4 a) : x((float)a.x), y((float)a.y), z((float)a.z), w((float)a.w) {}
		float4(const int4 a) : x((float)a.x), y((float)a.y), z((float)a.z), w((float)a.w) {}
		union { struct { float x, y, z, w; }; float cell[4]; };
		float& operator [] (const int n) { return cell[n]; }
	};

	struct float3 {
		float3() = default;
		float3(const float a, const float b, const float c) : x(a), y(b), z(c) {}
		float3(const float a) : x(a), y(a), z(a) {}
		float3(const float4 a) : x(a.x), y(a.y), z(a.z) {}
		float3(const uint3 a) : x((float)a.x), y((float)a.y), z((float)a.z) {}
		float3(const int3 a) : x((float)a.x), y((float)a.y), z((float)a.z) {}
		float2 xy() { return float2(x, y); }
		float2 yz() { return float2(y, z); }
		union { struct { float x, y, z; }; float cell[3]; };
		float& operator [] (const int n) { return cell[n]; }
		bool isZero() const { return x == 0 && y == 0 && z == 0; }
		//magnitude
		float magnitude() { return sqrt(x * x + y * y + z * z); }
	};
	struct ALIGN(4) uchar4 {
		uchar4() = default;
		uchar4(const uchar a, const uchar b, const uchar c, const uchar d) : x(a), y(b), z(c), w(d) {}
		uchar4(const uchar a) : x(a), y(a), z(a), w(a) {}
		union { struct { uchar x, y, z, w; }; uchar cell[4]; };
		uchar& operator [] (const int n) { return cell[n]; }
	};

#pragma warning ( pop )

}

// swap
template <class T> void Swap(T& x, T& y) { T t; t = x, x = y, y = t; }

// random numbers
uint InitSeed(uint seedBase);
uint RandomUInt();
uint RandomUInt(uint& seed);
float RandomFloat();
float RandomFloat(float min, float max);
float RandomFloat(uint& seed);
float Rand(float range);

// math
inline float fminf(const float a, const float b) { return a < b ? a : b; }
inline float fmaxf(const float a, const float b) { return a > b ? a : b; }
inline float rsqrtf(const float x) { return 1.0f / sqrtf(x); }
inline constexpr float sqrf(const float x) { return x * x; }
inline constexpr int sqr(int x) { return x * x; }
inline float3 expf(const float3& a) { return float3(expf(a.x), expf(a.y), expf(a.z)); }

inline float2 make_float2(const float a, float b) { float2 f2; f2.x = a, f2.y = b; return f2; }
inline float2 make_float2(const float s) { return make_float2(s, s); }
inline float2 make_float2(const float3& a) { return make_float2(a.x, a.y); }
inline float2 make_float2(const int2& a) { return make_float2(float(a.x), float(a.y)); } // explicit casts prevent gcc warnings
inline float2 make_float2(const uint2& a) { return make_float2(float(a.x), float(a.y)); }
inline int2 make_int2(const int a, const int b) { int2 i2; i2.x = a, i2.y = b; return i2; }
inline int2 make_int2(const int s) { return make_int2(s, s); }
inline int2 make_int2(const int3& a) { return make_int2(a.x, a.y); }
inline int2 make_int2(const uint2& a) { return make_int2(int(a.x), int(a.y)); }
inline int2 make_int2(const float2& a) { return make_int2(int(a.x), int(a.y)); }
inline uint2 make_uint2(const uint a, const uint b) { uint2 u2; u2.x = a, u2.y = b; return u2; }
inline uint2 make_uint2(const uint s) { return make_uint2(s, s); }
inline uint2 make_uint2(const uint3& a) { return make_uint2(a.x, a.y); }
inline uint2 make_uint2(const int2& a) { return make_uint2(uint(a.x), uint(a.y)); }
inline float3 make_float3(const float& a, const float& b, const float& c) { float3 f3; f3.x = a, f3.y = b, f3.z = c; return f3; }
inline float3 make_float3(const float& s) { return make_float3(s, s, s); }
inline float3 make_float3(const float2& a) { return make_float3(a.x, a.y, 0.0f); }
inline float3 make_float3(const float2& a, const float& s) { return make_float3(a.x, a.y, s); }
inline float3 make_float3(const float4& a) { return make_float3(a.x, a.y, a.z); }
inline float3 make_float3(const int3& a) { return make_float3(float(a.x), float(a.y), float(a.z)); }
inline float3 make_float3(const uint3& a) { return make_float3(float(a.x), float(a.y), float(a.z)); }
inline int3 make_int3(const int& a, const int& b, const int& c) { int3 i3; i3.x = a, i3.y = b, i3.z = c; return i3; }
inline int3 make_int3(const int& s) { return make_int3(s, s, s); }
inline int3 make_int3(const int2& a) { return make_int3(a.x, a.y, 0); }
inline int3 make_int3(const int2& a, const int& s) { return make_int3(a.x, a.y, s); }
inline int3 make_int3(const uint3& a) { return make_int3(int(a.x), int(a.y), int(a.z)); }
inline int3 make_int3(const float3& a) { return make_int3(int(a.x), int(a.y), int(a.z)); }
inline int3 make_int3(const float4& a) { return make_int3(int(a.x), int(a.y), int(a.z)); }
inline uint3 make_uint3(const uint a, uint b, uint c) { uint3 u3; u3.x = a, u3.y = b, u3.z = c; return u3; }
inline uint3 make_uint3(const uint s) { return make_uint3(s, s, s); }
inline uint3 make_uint3(const uint2& a) { return make_uint3(a.x, a.y, 0); }
inline uint3 make_uint3(const uint2& a, const uint s) { return make_uint3(a.x, a.y, s); }
inline uint3 make_uint3(const uint4& a) { return make_uint3(a.x, a.y, a.z); }
inline uint3 make_uint3(const int3& a) { return make_uint3(uint(a.x), uint(a.y), uint(a.z)); }
inline float4 make_float4(const float a, const float b, const float c, const float d) { float4 f4; f4.x = a, f4.y = b, f4.z = c, f4.w = d; return f4; }
inline float4 make_float4(const float s) { return make_float4(s, s, s, s); }
inline float4 make_float4(const float3& a) { return make_float4(a.x, a.y, a.z, 0.0f); }
inline float4 make_float4(const float3& a, const float w) { return make_float4(a.x, a.y, a.z, w); }
inline float4 make_float4(const int3& a, const float w) { return make_float4((float)a.x, (float)a.y, (float)a.z, w); }
inline float4 make_float4(const int4& a) { return make_float4(float(a.x), float(a.y), float(a.z), float(a.w)); }
inline float4 make_float4(const uint4& a) { return make_float4(float(a.x), float(a.y), float(a.z), float(a.w)); }
inline int4 make_int4(const int a, const int b, const int c, const int d) { int4 i4; i4.x = a, i4.y = b, i4.z = c, i4.w = d; return i4; }
inline int4 make_int4(const int s) { return make_int4(s, s, s, s); }
inline int4 make_int4(const int3& a) { return make_int4(a.x, a.y, a.z, 0); }
inline int4 make_int4(const int3& a, const int w) { return make_int4(a.x, a.y, a.z, w); }
inline int4 make_int4(const uint4& a) { return make_int4(int(a.x), int(a.y), int(a.z), int(a.w)); }
inline int4 make_int4(const float4& a) { return make_int4(int(a.x), int(a.y), int(a.z), int(a.w)); }
inline uint4 make_uint4(const uint a, const uint b, const uint c, const uint d) { uint4 u4; u4.x = a, u4.y = b, u4.z = c, u4.w = d; return u4; }
inline uint4 make_uint4(const uint s) { return make_uint4(s, s, s, s); }
inline uint4 make_uint4(const uint3& a) { return make_uint4(a.x, a.y, a.z, 0); }
inline uint4 make_uint4(const uint3& a, const uint w) { return make_uint4(a.x, a.y, a.z, w); }
inline uint4 make_uint4(const int4& a) { return make_uint4(uint(a.x), uint(a.y), uint(a.z), uint(a.w)); }
inline uchar4 make_uchar4(const uchar a, const uchar b, const uchar c, const uchar d) { uchar4 c4; c4.x = a, c4.y = b, c4.z = c, c4.w = d; return c4; }

inline float2 operator-(const float2& a) { return make_float2(-a.x, -a.y); }
inline int2 operator-(const int2& a) { return make_int2(-a.x, -a.y); }
inline float3 operator-(const float3& a) { return make_float3(-a.x, -a.y, -a.z); }
inline int3 operator-(const int3& a) { return make_int3(-a.x, -a.y, -a.z); }
inline float4 operator-(const float4& a) { return make_float4(-a.x, -a.y, -a.z, -a.w); }
inline int4 operator-(const int4& a) { return make_int4(-a.x, -a.y, -a.z, -a.w); }
inline int2 operator << (const int2& a, int b) { return make_int2(a.x << b, a.y << b); }
inline int2 operator >> (const int2& a, int b) { return make_int2(a.x >> b, a.y >> b); }
inline int3 operator << (const int3& a, int b) { return make_int3(a.x << b, a.y << b, a.z << b); }
inline int3 operator >> (const int3& a, int b) { return make_int3(a.x >> b, a.y >> b, a.z >> b); }
inline int4 operator << (const int4& a, int b) { return make_int4(a.x << b, a.y << b, a.z << b, a.w << b); }
inline int4 operator >> (const int4& a, int b) { return make_int4(a.x >> b, a.y >> b, a.z >> b, a.w >> b); }

inline float2 operator+(const float2& a, const float2& b) { return make_float2(a.x + b.x, a.y + b.y); }
inline float2 operator+(const float2& a, const int2& b) { return make_float2(a.x + (float)b.x, a.y + (float)b.y); }
inline float2 operator+(const float2& a, const uint2& b) { return make_float2(a.x + (float)b.x, a.y + (float)b.y); }
inline float2 operator+(const int2& a, const float2& b) { return make_float2((float)a.x + b.x, (float)a.y + b.y); }
inline float2 operator+(const uint2& a, const float2& b) { return make_float2((float)a.x + b.x, (float)a.y + b.y); }
inline void operator+=(float2& a, const float2& b) { a.x += b.x;	a.y += b.y; }
inline void operator+=(float2& a, const int2& b) { a.x += (float)b.x; a.y += (float)b.y; }
inline void operator+=(float2& a, const uint2& b) { a.x += (float)b.x; a.y += (float)b.y; }
inline float2 operator+(const float2& a, float b) { return make_float2(a.x + b, a.y + b); }
inline float2 operator+(const float2& a, int b) { return make_float2(a.x + (float)b, a.y + (float)b); }
inline float2 operator+(const float2& a, uint b) { return make_float2(a.x + (float)b, a.y + (float)b); }
inline float2 operator+(float b, const float2& a) { return make_float2(a.x + b, a.y + b); }
inline void operator+=(float2& a, float b) { a.x += b; a.y += b; }
inline void operator+=(float2& a, int b) { a.x += (float)b; a.y += (float)b; }
inline void operator+=(float2& a, uint b) { a.x += (float)b;	a.y += (float)b; }
inline int2 operator+(const int2& a, const int2& b) { return make_int2(a.x + b.x, a.y + b.y); }
inline void operator+=(int2& a, const int2& b) { a.x += b.x;	a.y += b.y; }
inline int2 operator+(const int2& a, int b) { return make_int2(a.x + b, a.y + b); }
inline int2 operator+(int b, const int2& a) { return make_int2(a.x + b, a.y + b); }
inline void operator+=(int2& a, int b) { a.x += b;	a.y += b; }
inline uint2 operator+(const uint2& a, const uint2& b) { return make_uint2(a.x + b.x, a.y + b.y); }
inline void operator+=(uint2& a, const uint2& b) { a.x += b.x;	a.y += b.y; }
inline uint2 operator+(const uint2& a, uint b) { return make_uint2(a.x + b, a.y + b); }
inline uint2 operator+(uint b, const uint2& a) { return make_uint2(a.x + b, a.y + b); }
inline void operator+=(uint2& a, uint b) { a.x += b;	a.y += b; }
inline float3 operator+(const float3& a, const float3& b) { return make_float3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline float3 operator+(const float3& a, const int3& b) { return make_float3(a.x + (float)b.x, a.y + (float)b.y, a.z + (float)b.z); }
inline float3 operator+(const float3& a, const uint3& b) { return make_float3(a.x + (float)b.x, a.y + (float)b.y, a.z + (float)b.z); }
inline float3 operator+(const int3& a, const float3& b) { return make_float3((float)a.x + b.x, (float)a.y + b.y, (float)a.z + b.z); }
inline float3 operator+(const uint3& a, const float3& b) { return make_float3((float)a.x + b.x, (float)a.y + b.y, (float)a.z + b.z); }
inline void operator+=(float3& a, const float3& b) { a.x += b.x;	a.y += b.y;	a.z += b.z; }
inline void operator+=(float3& a, const int3& b) { a.x += (float)b.x; a.y += (float)b.y; a.z += (float)b.z; }
inline void operator+=(float3& a, const uint3& b) { a.x += (float)b.x; a.y += (float)b.y; a.z += (float)b.z; }
inline float3 operator+(const float3& a, float b) { return make_float3(a.x + b, a.y + b, a.z + b); }
inline float3 operator+(const float3& a, int b) { return make_float3(a.x + (float)b, a.y + (float)b, a.z + (float)b); }
inline float3 operator+(const float3& a, uint b) { return make_float3(a.x + (float)b, a.y + (float)b, a.z + (float)b); }
inline void operator+=(float3& a, float b) { a.x += b; a.y += b;	a.z += b; }
inline void operator+=(float3& a, int b) { a.x += (float)b; a.y += (float)b; a.z += (float)b; }
inline void operator+=(float3& a, uint b) { a.x += (float)b; a.y += (float)b; a.z += (float)b; }
inline int3 operator+(const int3& a, const int3& b) { return make_int3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline void operator+=(int3& a, const int3& b) { a.x += b.x;	a.y += b.y;	a.z += b.z; }
inline int3 operator+(const int3& a, int b) { return make_int3(a.x + b, a.y + b, a.z + b); }
inline void operator+=(int3& a, int b) { a.x += b;	a.y += b;	a.z += b; }
inline uint3 operator+(const uint3& a, const uint3& b) { return make_uint3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline void operator+=(uint3& a, const uint3& b) { a.x += b.x;	a.y += b.y;	a.z += b.z; }
inline uint3 operator+(const uint3& a, uint b) { return make_uint3(a.x + b, a.y + b, a.z + b); }
inline void operator+=(uint3& a, uint b) { a.x += b;	a.y += b;	a.z += b; }
inline int3 operator+(int b, const int3& a) { return make_int3(a.x + b, a.y + b, a.z + b); }
inline uint3 operator+(uint b, const uint3& a) { return make_uint3(a.x + b, a.y + b, a.z + b); }
inline float3 operator+(float b, const float3& a) { return make_float3(a.x + b, a.y + b, a.z + b); }
inline float4 operator+(const float4& a, const float4& b) { return make_float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
inline float4 operator+(const float4& a, const int4& b) { return make_float4(a.x + (float)b.x, a.y + (float)b.y, a.z + (float)b.z, a.w + (float)b.w); }
inline float4 operator+(const float4& a, const uint4& b) { return make_float4(a.x + (float)b.x, a.y + (float)b.y, a.z + (float)b.z, a.w + (float)b.w); }
inline float4 operator+(const int4& a, const float4& b) { return make_float4((float)a.x + b.x, (float)a.y + b.y, (float)a.z + b.z, (float)a.w + b.w); }
inline float4 operator+(const uint4& a, const float4& b) { return make_float4((float)a.x + b.x, (float)a.y + b.y, (float)a.z + b.z, (float)a.w + b.w); }
inline void operator+=(float4& a, const float4& b) { a.x += b.x;	a.y += b.y;	a.z += b.z;	a.w += b.w; }
inline void operator+=(float4& a, const float3& b) { a.x += b.x;	a.y += b.y;	a.z += b.z; }
inline void operator+=(float4& a, const int4& b) { a.x += (float)b.x; a.y += (float)b.y; a.z += (float)b.z; a.w += (float)b.w; }
inline void operator+=(float4& a, const uint4& b) { a.x += (float)b.x; a.y += (float)b.y; a.z += (float)b.z; a.w += (float)b.w; }
inline float4 operator+(const float4& a, float b) { return make_float4(a.x + b, a.y + b, a.z + b, a.w + b); }
inline float4 operator+(const float4& a, int b) { return make_float4(a.x + (float)b, a.y + (float)b, a.z + (float)b, a.w + (float)b); }
inline float4 operator+(const float4& a, uint b) { return make_float4(a.x + (float)b, a.y + (float)b, a.z + (float)b, a.w + (float)b); }
inline float4 operator+(float b, const float4& a) { return make_float4(a.x + b, a.y + b, a.z + b, a.w + b); }
inline void operator+=(float4& a, float b) { a.x += b;	a.y += b;	a.z += b;	a.w += b; }
inline void operator+=(float4& a, int b) { a.x += (float)b; a.y += (float)b; a.z += (float)b; a.w += (float)b; }
inline void operator+=(float4& a, uint b) { a.x += (float)b; a.y += (float)b; a.z += (float)b; a.w += (float)b; }
inline int4 operator+(const int4& a, const int4& b) { return make_int4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
inline void operator+=(int4& a, const int4& b) { a.x += b.x;	a.y += b.y;	a.z += b.z;	a.w += b.w; }
inline int4 operator+(const int4& a, int b) { return make_int4(a.x + b, a.y + b, a.z + b, a.w + b); }
inline int4 operator+(int b, const int4& a) { return make_int4(a.x + b, a.y + b, a.z + b, a.w + b); }
inline void operator+=(int4& a, int b) { a.x += b;	a.y += b;	a.z += b;	a.w += b; }
inline uint4 operator+(const uint4& a, const uint4& b) { return make_uint4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
inline void operator+=(uint4& a, const uint4& b) { a.x += b.x;	a.y += b.y;	a.z += b.z;	a.w += b.w; }
inline uint4 operator+(const uint4& a, uint b) { return make_uint4(a.x + b, a.y + b, a.z + b, a.w + b); }
inline uint4 operator+(uint b, const uint4& a) { return make_uint4(a.x + b, a.y + b, a.z + b, a.w + b); }
inline void operator+=(uint4& a, uint b) { a.x += b;	a.y += b;	a.z += b;	a.w += b; }

inline float2 operator-(const float2& a, const float2& b) { return make_float2(a.x - b.x, a.y - b.y); }
inline float2 operator-(const float2& a, const int2& b) { return make_float2(a.x - (float)b.x, a.y - (float)b.y); }
inline float2 operator-(const float2& a, const uint2& b) { return make_float2(a.x - (float)b.x, a.y - (float)b.y); }
inline float2 operator-(const int2& a, const float2& b) { return make_float2((float)a.x - b.x, (float)a.y - b.y); }
inline float2 operator-(const uint2& a, const float2& b) { return make_float2((float)a.x - b.x, (float)a.y - b.y); }
inline void operator-=(float2& a, const float2& b) { a.x -= b.x;	a.y -= b.y; }
inline void operator-=(float2& a, const int2& b) { a.x -= (float)b.x; a.y -= (float)b.y; }
inline void operator-=(float2& a, const uint2& b) { a.x -= (float)b.x; a.y -= (float)b.y; }
inline float2 operator-(const float2& a, float b) { return make_float2(a.x - b, a.y - b); }
inline float2 operator-(const float2& a, int b) { return make_float2(a.x - (float)b, a.y - (float)b); }
inline float2 operator-(const float2& a, uint b) { return make_float2(a.x - (float)b, a.y - (float)b); }
inline float2 operator-(float b, const float2& a) { return make_float2(b - a.x, b - a.y); }
inline void operator-=(float2& a, float b) { a.x -= b; a.y -= b; }
inline void operator-=(float2& a, int b) { a.x -= (float)b; a.y -= (float)b; }
inline void operator-=(float2& a, uint b) { a.x -= (float)b; a.y -= (float)b; }
inline int2 operator-(const int2& a, const int2& b) { return make_int2(a.x - b.x, a.y - b.y); }
inline void operator-=(int2& a, const int2& b) { a.x -= b.x;	a.y -= b.y; }
inline int2 operator-(const int2& a, int b) { return make_int2(a.x - b, a.y - b); }
inline int2 operator-(int b, const int2& a) { return make_int2(b - a.x, b - a.y); }
inline void operator-=(int2& a, int b) { a.x -= b;	a.y -= b; }
inline uint2 operator-(const uint2& a, const uint2& b) { return make_uint2(a.x - b.x, a.y - b.y); }
inline void operator-=(uint2& a, const uint2& b) { a.x -= b.x;	a.y -= b.y; }
inline uint2 operator-(const uint2& a, uint b) { return make_uint2(a.x - b, a.y - b); }
inline uint2 operator-(uint b, const uint2& a) { return make_uint2(b - a.x, b - a.y); }
inline void operator-=(uint2& a, uint b) { a.x -= b;	a.y -= b; }
inline float3 operator-(const float3& a, const float3& b) { return make_float3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline float3 operator-(const float3& a, const int3& b) { return make_float3(a.x - (float)b.x, a.y - (float)b.y, a.z - (float)b.z); }
inline float3 operator-(const float3& a, const uint3& b) { return make_float3(a.x - (float)b.x, a.y - (float)b.y, a.z - (float)b.z); }
inline float3 operator-(const int3& a, const float3& b) { return make_float3((float)a.x - b.x, (float)a.y - b.y, (float)a.z - b.z); }
inline float3 operator-(const uint3& a, const float3& b) { return make_float3((float)a.x - b.x, (float)a.y - b.y, (float)a.z - b.z); }
inline void operator-=(float3& a, const float3& b) { a.x -= b.x;	a.y -= b.y;	a.z -= b.z; }
inline void operator-=(float3& a, const int3& b) { a.x -= (float)b.x; a.y -= (float)b.y; a.z -= (float)b.z; }
inline void operator-=(float3& a, const uint3& b) { a.x -= (float)b.x; a.y -= (float)b.y; a.z -= (float)b.z; }
inline float3 operator-(const float3& a, float b) { return make_float3(a.x - b, a.y - b, a.z - b); }
inline float3 operator-(const float3& a, int b) { return make_float3(a.x - (float)b, a.y - (float)b, a.z - (float)b); }
inline float3 operator-(const float3& a, uint b) { return make_float3(a.x - (float)b, a.y - (float)b, a.z - (float)b); }
inline float3 operator-(float b, const float3& a) { return make_float3(b - a.x, b - a.y, b - a.z); }
inline void operator-=(float3& a, float b) { a.x -= b; a.y -= b; a.z -= b; }
inline void operator-=(float3& a, int b) { a.x -= (float)b; a.y -= (float)b; a.z -= (float)b; }
inline void operator-=(float3& a, uint b) { a.x -= (float)b;	a.y -= (float)b; a.z -= (float)b; }
inline int3 operator-(const int3& a, const int3& b) { return make_int3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline void operator-=(int3& a, const int3& b) { a.x -= b.x;	a.y -= b.y;	a.z -= b.z; }
inline int3 operator-(const int3& a, int b) { return make_int3(a.x - b, a.y - b, a.z - b); }
inline int3 operator-(int b, const int3& a) { return make_int3(b - a.x, b - a.y, b - a.z); }
inline void operator-=(int3& a, int b) { a.x -= b;	a.y -= b;	a.z -= b; }
inline uint3 operator-(const uint3& a, const uint3& b) { return make_uint3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline void operator-=(uint3& a, const uint3& b) { a.x -= b.x;	a.y -= b.y;	a.z -= b.z; }
inline uint3 operator-(const uint3& a, uint b) { return make_uint3(a.x - b, a.y - b, a.z - b); }
inline uint3 operator-(uint b, const uint3& a) { return make_uint3(b - a.x, b - a.y, b - a.z); }
inline void operator-=(uint3& a, uint b) { a.x -= b;	a.y -= b;	a.z -= b; }
inline float4 operator-(const float4& a, const float4& b) { return make_float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
inline float4 operator-(const float4& a, const int4& b) { return make_float4(a.x - (float)b.x, a.y - (float)b.y, a.z - (float)b.z, a.w - (float)b.w); }
inline float4 operator-(const float4& a, const uint4& b) { return make_float4(a.x - (float)b.x, a.y - (float)b.y, a.z - (float)b.z, a.w - (float)b.w); }
inline float4 operator-(const int4& a, const float4& b) { return make_float4((float)a.x - b.x, (float)a.y - b.y, (float)a.z - b.z, (float)a.w - b.w); }
inline float4 operator-(const uint4& a, const float4& b) { return make_float4((float)a.x - b.x, (float)a.y - b.y, (float)a.z - b.z, (float)a.w - b.w); }
inline void operator-=(float4& a, const float4& b) { a.x -= b.x;	a.y -= b.y;	a.z -= b.z;	a.w -= b.w; }
inline void operator-=(float4& a, const int4& b) { a.x -= (float)b.x; a.y -= (float)b.y; a.z -= (float)b.z; a.w -= (float)b.w; }
inline void operator-=(float4& a, const uint4& b) { a.x -= (float)b.x; a.y -= (float)b.y; a.z -= (float)b.z; a.w -= (float)b.w; }
inline float4 operator-(const float4& a, float b) { return make_float4(a.x - b, a.y - b, a.z - b, a.w - b); }
inline float4 operator-(const float4& a, int b) { return make_float4(a.x - (float)b, a.y - (float)b, a.z - (float)b, a.w - (float)b); }
inline float4 operator-(const float4& a, uint b) { return make_float4(a.x - (float)b, a.y - (float)b, a.z - (float)b, a.w - (float)b); }
inline void operator-=(float4& a, float b) { a.x -= b; a.y -= b; a.z -= b; a.w -= b; }
inline void operator-=(float4& a, int b) { a.x -= (float)b; a.y -= (float)b; a.z -= (float)b; a.w -= (float)b; }
inline void operator-=(float4& a, uint b) { a.x -= (float)b; a.y -= (float)b; a.z -= (float)b; a.w -= (float)b; }
inline int4 operator-(const int4& a, const int4& b) { return make_int4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
inline void operator-=(int4& a, const int4& b) { a.x -= b.x;	a.y -= b.y;	a.z -= b.z;	a.w -= b.w; }
inline int4 operator-(const int4& a, int b) { return make_int4(a.x - b, a.y - b, a.z - b, a.w - b); }
inline int4 operator-(int b, const int4& a) { return make_int4(b - a.x, b - a.y, b - a.z, b - a.w); }
inline void operator-=(int4& a, int b) { a.x -= b;	a.y -= b;	a.z -= b;	a.w -= b; }
inline uint4 operator-(const uint4& a, const uint4& b) { return make_uint4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
inline void operator-=(uint4& a, const uint4& b) { a.x -= b.x;	a.y -= b.y;	a.z -= b.z;	a.w -= b.w; }
inline uint4 operator-(const uint4& a, uint b) { return make_uint4(a.x - b, a.y - b, a.z - b, a.w - b); }
inline uint4 operator-(uint b, const uint4& a) { return make_uint4(b - a.x, b - a.y, b - a.z, b - a.w); }
inline void operator-=(uint4& a, uint b) { a.x -= b;	a.y -= b;	a.z -= b;	a.w -= b; }

inline float2 operator*(const float2& a, const float2& b) { return make_float2(a.x * b.x, a.y * b.y); }
inline void operator*=(float2& a, const float2& b) { a.x *= b.x;	a.y *= b.y; }
inline float2 operator*(const float2& a, float b) { return make_float2(a.x * b, a.y * b); }
inline float2 operator*(float b, const float2& a) { return make_float2(b * a.x, b * a.y); }
inline void operator*=(float2& a, float b) { a.x *= b;	a.y *= b; }
inline int2 operator*(const int2& a, const int2& b) { return make_int2(a.x * b.x, a.y * b.y); }
inline void operator*=(int2& a, const int2& b) { a.x *= b.x;	a.y *= b.y; }
inline int2 operator*(const int2& a, int b) { return make_int2(a.x * b, a.y * b); }
inline int2 operator*(int b, const int2& a) { return make_int2(b * a.x, b * a.y); }
inline void operator*=(int2& a, int b) { a.x *= b;	a.y *= b; }
inline uint2 operator*(const uint2& a, const uint2& b) { return make_uint2(a.x * b.x, a.y * b.y); }
inline void operator*=(uint2& a, const uint2& b) { a.x *= b.x;	a.y *= b.y; }
inline uint2 operator*(const uint2& a, uint b) { return make_uint2(a.x * b, a.y * b); }
inline uint2 operator*(uint b, const uint2& a) { return make_uint2(b * a.x, b * a.y); }
inline void operator*=(uint2& a, uint b) { a.x *= b;	a.y *= b; }
inline float3 operator*(const float3& a, const float3& b) { return make_float3(a.x * b.x, a.y * b.y, a.z * b.z); }
inline void operator*=(float3& a, const float3& b) { a.x *= b.x;	a.y *= b.y;	a.z *= b.z; }
inline float3 operator*(const float3& a, float b) { return make_float3(a.x * b, a.y * b, a.z * b); }
inline float3 operator*(float b, const float3& a) { return make_float3(b * a.x, b * a.y, b * a.z); }
inline void operator*=(float3& a, float b) { a.x *= b;	a.y *= b;	a.z *= b; }
inline int3 operator*(const int3& a, const int3& b) { return make_int3(a.x * b.x, a.y * b.y, a.z * b.z); }
inline void operator*=(int3& a, const int3& b) { a.x *= b.x;	a.y *= b.y;	a.z *= b.z; }
inline int3 operator*(const int3& a, int b) { return make_int3(a.x * b, a.y * b, a.z * b); }
inline int3 operator*(int b, const int3& a) { return make_int3(b * a.x, b * a.y, b * a.z); }
inline void operator*=(int3& a, int b) { a.x *= b;	a.y *= b;	a.z *= b; }
inline uint3 operator*(const uint3& a, const uint3& b) { return make_uint3(a.x * b.x, a.y * b.y, a.z * b.z); }
inline void operator*=(uint3& a, const uint3& b) { a.x *= b.x;	a.y *= b.y;	a.z *= b.z; }
inline uint3 operator*(const uint3& a, uint b) { return make_uint3(a.x * b, a.y * b, a.z * b); }
inline uint3 operator*(uint b, const uint3& a) { return make_uint3(b * a.x, b * a.y, b * a.z); }
inline void operator*=(uint3& a, uint b) { a.x *= b;	a.y *= b;	a.z *= b; }
inline float4 operator*(const float4& a, const float4& b) { return make_float4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
inline void operator*=(float4& a, const float4& b) { a.x *= b.x;	a.y *= b.y;	a.z *= b.z;	a.w *= b.w; }
inline float4 operator*(const float4& a, float b) { return make_float4(a.x * b, a.y * b, a.z * b, a.w * b); }
inline float4 operator*(float b, const float4& a) { return make_float4(b * a.x, b * a.y, b * a.z, b * a.w); }
inline void operator*=(float4& a, float b) { a.x *= b;	a.y *= b;	a.z *= b;	a.w *= b; }
inline int4 operator*(const int4& a, const int4& b) { return make_int4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
inline void operator*=(int4& a, const int4& b) { a.x *= b.x;	a.y *= b.y;	a.z *= b.z;	a.w *= b.w; }
inline int4 operator*(const int4& a, int b) { return make_int4(a.x * b, a.y * b, a.z * b, a.w * b); }
inline int4 operator*(int b, const int4& a) { return make_int4(b * a.x, b * a.y, b * a.z, b * a.w); }
inline void operator*=(int4& a, int b) { a.x *= b;	a.y *= b;	a.z *= b;	a.w *= b; }
inline uint4 operator*(const uint4& a, const uint4& b) { return make_uint4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
inline void operator*=(uint4& a, const uint4& b) { a.x *= b.x;	a.y *= b.y;	a.z *= b.z;	a.w *= b.w; }
inline uint4 operator*(const uint4& a, uint b) { return make_uint4(a.x * b, a.y * b, a.z * b, a.w * b); }
inline uint4 operator*(uint b, const uint4& a) { return make_uint4(b * a.x, b * a.y, b * a.z, b * a.w); }
inline void operator*=(uint4& a, uint b) { a.x *= b;	a.y *= b;	a.z *= b;	a.w *= b; }

inline float2 operator/(const float2& a, const float2& b) { return make_float2(a.x / b.x, a.y / b.y); }
inline void operator/=(float2& a, const float2& b) { a.x /= b.x;	a.y /= b.y; }
inline float2 operator/(const float2& a, float b) { return make_float2(a.x / b, a.y / b); }
inline void operator/=(float2& a, float b) { a.x /= b;	a.y /= b; }
inline float2 operator/(float b, const float2& a) { return make_float2(b / a.x, b / a.y); }
inline float3 operator/(const float3& a, const float3& b) { return make_float3(a.x / b.x, a.y / b.y, a.z / b.z); }
inline void operator/=(float3& a, const float3& b) { a.x /= b.x;	a.y /= b.y;	a.z /= b.z; }
inline float3 operator/(const float3& a, float b) { return make_float3(a.x / b, a.y / b, a.z / b); }
inline void operator/=(float3& a, float b) { a.x /= b;	a.y /= b;	a.z /= b; }
inline float3 operator/(float b, const float3& a) { return make_float3(b / a.x, b / a.y, b / a.z); }
inline float4 operator/(const float4& a, const float4& b) { return make_float4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }
inline void operator/=(float4& a, const float4& b) { a.x /= b.x;	a.y /= b.y;	a.z /= b.z;	a.w /= b.w; }
inline float4 operator/(const float4& a, float b) { return make_float4(a.x / b, a.y / b, a.z / b, a.w / b); }
inline void operator/=(float4& a, float b) { a.x /= b;	a.y /= b;	a.z /= b;	a.w /= b; }
inline float4 operator/(float b, const float4& a) { return make_float4(b / a.x, b / a.y, b / a.z, b / a.w); }

inline float2 fminf(const float2& a, const float2& b) { return make_float2(fminf(a.x, b.x), fminf(a.y, b.y)); }
inline float3 fminf(const float3& a, const float3& b) { return make_float3(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z)); }
inline float4 fminf(const float4& a, const float4& b) { return make_float4(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z), fminf(a.w, b.w)); }
inline int2 min(const int2& a, const int2& b) { return make_int2(min(a.x, b.x), min(a.y, b.y)); }
inline int3 min(const int3& a, const int3& b) { return make_int3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z)); }
inline int4 min(const int4& a, const int4& b) { return make_int4(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z), min(a.w, b.w)); }
inline uint2 min(const uint2& a, const uint2& b) { return make_uint2(min(a.x, b.x), min(a.y, b.y)); }
inline uint3 min(const uint3& a, const uint3& b) { return make_uint3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z)); }
inline uint4 min(const uint4& a, const uint4& b) { return make_uint4(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z), min(a.w, b.w)); }

inline float2 fmaxf(const float2& a, const float2& b) { return make_float2(fmaxf(a.x, b.x), fmaxf(a.y, b.y)); }
inline float3 fmaxf(const float3& a, const float3& b) { return make_float3(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z)); }
inline float4 fmaxf(const float4& a, const float4& b) { return make_float4(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z), fmaxf(a.w, b.w)); }
inline int2 max(const int2& a, const int2& b) { return make_int2(max(a.x, b.x), max(a.y, b.y)); }
inline int3 max(const int3& a, const int3& b) { return make_int3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z)); }
inline int4 max(const int4& a, const int4& b) { return make_int4(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z), max(a.w, b.w)); }
inline uint2 max(const uint2& a, const uint2& b) { return make_uint2(max(a.x, b.x), max(a.y, b.y)); }
inline uint3 max(const uint3& a, const uint3& b) { return make_uint3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z)); }
inline uint4 max(const uint4& a, const uint4& b) { return make_uint4(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z), max(a.w, b.w)); }

inline float clamp(float f, float a, float b) { return fmaxf(a, fminf(f, b)); }
inline int clamp(int f, int a, int b) { return max(a, min(f, b)); }
inline uint clamp(uint f, uint a, uint b) { return max(a, min(f, b)); }
inline float2 clamp(const float2& v, float a, float b) { return make_float2(clamp(v.x, a, b), clamp(v.y, a, b)); }
inline float2 clamp(const float2& v, const float2& a, const float2& b) { return make_float2(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y)); }
inline float3 clamp(const float3& v, float a, float b) { return make_float3(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b)); }
inline float3 clamp(const float3& v, const float3& a, const float3& b) { return make_float3(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z)); }
inline float4 clamp(const float4& v, float a, float b) { return make_float4(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b), clamp(v.w, a, b)); }
inline float4 clamp(const float4& v, const float4& a, const float4& b) { return make_float4(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z), clamp(v.w, a.w, b.w)); }
inline int2 clamp(const int2& v, int a, int b) { return make_int2(clamp(v.x, a, b), clamp(v.y, a, b)); }
inline int2 clamp(const int2& v, const int2& a, const int2& b) { return make_int2(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y)); }
inline int3 clamp(const int3& v, int a, int b) { return make_int3(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b)); }
inline int3 clamp(const int3& v, const int3& a, const int3& b) { return make_int3(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z)); }
inline int4 clamp(const int4& v, int a, int b) { return make_int4(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b), clamp(v.w, a, b)); }
inline int4 clamp(const int4& v, const int4& a, const int4& b) { return make_int4(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z), clamp(v.w, a.w, b.w)); }
inline uint2 clamp(const uint2& v, uint a, uint b) { return make_uint2(clamp(v.x, a, b), clamp(v.y, a, b)); }
inline uint2 clamp(const uint2& v, const uint2& a, const uint2& b) { return make_uint2(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y)); }
inline uint3 clamp(const uint3& v, uint a, uint b) { return make_uint3(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b)); }
inline uint3 clamp(const uint3& v, const uint3& a, const uint3& b) { return make_uint3(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z)); }
inline uint4 clamp(const uint4& v, uint a, uint b) { return make_uint4(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b), clamp(v.w, a, b)); }
inline uint4 clamp(const uint4& v, const uint4& a, const uint4& b) { return make_uint4(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z), clamp(v.w, a.w, b.w)); }

inline float lerp(const float a, const float b, const float t) { return a + t * (b - a); }
inline float2 lerp(const float2& a, const float2& b, float t) { return a + t * (b - a); }
inline float3 lerp(const float3& a, const float3& b, float t) { return a + t * (b - a); }
inline float4 lerp(const float4& a, const float4& b, float t) { return a + t * (b - a); }

inline bool operator==(const float2& a, const float2& b) { return a.x == b.x && a.y == b.y; }
inline bool operator!=(const float2& a, const float2& b) { return a.x != b.x || a.y != b.y; }
inline bool operator==(const float3& a, const float3& b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
inline bool operator!=(const float3& a, const float3& b) { return a.x != b.x || a.y != b.y || a.z != b.z; }
inline bool operator==(const float4& a, const float4& b) { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }
inline bool operator!=(const float4& a, const float4& b) { return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w; }
inline bool operator==(const int2& a, const int2& b) { return a.x == b.x && a.y == b.y; }
inline bool operator!=(const int2& a, const int2& b) { return a.x != b.x || a.y != b.y; }
inline bool operator==(const int3& a, const int3& b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
inline bool operator!=(const int3& a, const int3& b) { return a.x != b.x || a.y != b.y || a.z != b.z; }
inline bool operator==(const int4& a, const int4& b) { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }
inline bool operator!=(const int4& a, const int4& b) { return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w; }

inline float smoothstep(const float a, const float b, const float x) {
	const float y = clamp((x - a) / (b - a), 0.0f, 1.0f);
	return (y * y * (3.0f - (2.0f * y)));
}
inline float2 smoothstep(const float2 a, const float2 b, const float2 x) {
	const float2 y = clamp((x - a) / (b - a), 0.0f, 1.0f);
	return (y * y * (make_float2(3.0f) - (make_float2(2.0f) * y)));
}
inline float3 smoothstep(const float3 a, const float3 b, const float3 x) {
	const float3 y = clamp((x - a) / (b - a), 0.0f, 1.0f);
	return (y * y * (make_float3(3.0f) - (make_float3(2.0f) * y)));
}
inline float4 smoothstep(const float4 a, const float4 b, const float4 x) {
	const float4 y = clamp((x - a) / (b - a), 0.0f, 1.0f);
	return (y * y * (make_float4(3.0f) - (make_float4(2.0f) * y)));
}

inline float dot(const float2& a, const float2& b) { return a.x * b.x + a.y * b.y; }
inline float dot(const float3& a, const float3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline float dot(const float4& a, const float4& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
inline int dot(const int2& a, const int2& b) { return a.x * b.x + a.y * b.y; }
inline int dot(const int3& a, const int3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline int dot(const int4& a, const int4& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
inline uint dot(const uint2& a, const uint2& b) { return a.x * b.x + a.y * b.y; }
inline uint dot(const uint3& a, const uint3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline uint dot(const uint4& a, const uint4& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

inline float sqrLength(const float2& v) { return dot(v, v); }
inline float sqrLength(const float3& v) { return dot(v, v); }
inline float sqrLength(const float4& v) { return dot(v, v); }

inline float length(const float2& v) { return sqrtf(dot(v, v)); }
inline float length(const float3& v) { return sqrtf(dot(v, v)); }
inline float length(const float4& v) { return sqrtf(dot(v, v)); }

inline float length(const int2& v) { return sqrtf((float)dot(v, v)); }
inline float length(const int3& v) { return sqrtf((float)dot(v, v)); }
inline float length(const int4& v) { return sqrtf((float)dot(v, v)); }

inline float2 normalize(const float2& v) { float invLen = rsqrtf(dot(v, v)); return v * invLen; }
inline float3 normalize(const float3& v) { float invLen = rsqrtf(dot(v, v)); return v * invLen; }
inline float4 normalize(const float4& v) { float invLen = rsqrtf(dot(v, v)); return v * invLen; }

inline uint dominantAxis(const float2& v) { float x = fabs(v.x), y = fabs(v.y); return x > y ? 0 : 1; } // for coherent grid traversal
inline uint dominantAxis(const float3& v) { float x = fabs(v.x), y = fabs(v.y), z = fabs(v.z); float m = max(max(x, y), z); return m == x ? 0 : (m == y ? 1 : 2); }

inline float2 floorf(const float2& v) { return make_float2(floorf(v.x), floorf(v.y)); }
inline float3 floorf(const float3& v) { return make_float3(floorf(v.x), floorf(v.y), floorf(v.z)); }
inline float4 floorf(const float4& v) { return make_float4(floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w)); }

inline int3 floori(const float3& v) {
	return make_int3(static_cast<int>(floorf(v.x)), static_cast<int>(floorf(v.y)), static_cast<int>(floorf(v.z)));
}

inline float2 ceilf(const float2& v) { return make_float2(ceilf(v.x), ceilf(v.y)); }
inline float3 ceilf(const float3& v) { return make_float3(ceilf(v.x), ceilf(v.y), ceilf(v.z)); }
inline float4 ceilf(const float4& v) { return make_float4(ceilf(v.x), ceilf(v.y), ceilf(v.z), ceilf(v.w)); }

inline float fracf(float v) { return v - floorf(v); }
inline float2 fracf(const float2& v) { return make_float2(fracf(v.x), fracf(v.y)); }
inline float3 fracf(const float3& v) { return make_float3(fracf(v.x), fracf(v.y), fracf(v.z)); }
inline float4 fracf(const float4& v) { return make_float4(fracf(v.x), fracf(v.y), fracf(v.z), fracf(v.w)); }

inline float2 fmodf(const float2& a, const float2& b) { return make_float2(fmodf(a.x, b.x), fmodf(a.y, b.y)); }
inline float3 fmodf(const float3& a, const float3& b) { return make_float3(fmodf(a.x, b.x), fmodf(a.y, b.y), fmodf(a.z, b.z)); }
inline float4 fmodf(const float4& a, const float4& b) { return make_float4(fmodf(a.x, b.x), fmodf(a.y, b.y), fmodf(a.z, b.z), fmodf(a.w, b.w)); }

inline float2 fabs(const float2& v) { return make_float2(fabs(v.x), fabs(v.y)); }
inline float3 fabs(const float3& v) { return make_float3(fabs(v.x), fabs(v.y), fabs(v.z)); }
inline float4 fabs(const float4& v) { return make_float4(fabs(v.x), fabs(v.y), fabs(v.z), fabs(v.w)); }
inline int2 abs(const int2& v) { return make_int2(abs(v.x), abs(v.y)); }
inline int3 abs(const int3& v) { return make_int3(abs(v.x), abs(v.y), abs(v.z)); }
inline int4 abs(const int4& v) { return make_int4(abs(v.x), abs(v.y), abs(v.z), abs(v.w)); }

inline float3 reflect(const float3& i, const float3& n) { return i - 2.0f * n * dot(n, i); }
//inline float3 refract(const float3& i, const float3& n, float eta) {
//	float k = 1.0f - eta * eta * (1.0f - dot(n, i) * dot(n, i));
//	if (k < 0.0f) return make_float3(0.0f);
//	else return eta * i - (eta * dot(n, i) + sqrtf(k)) * n;
//}

inline float3 refract(const float3& direction, const float3& normal, float IORRatio) {
	float cosi = dot(direction, normal);
	float cost2 = 1.0f - IORRatio * IORRatio * (1.0f - cosi * cosi);
	if (cost2 > 0.0f) {
		return IORRatio * direction - (IORRatio * cosi + sqrtf(cost2)) * normal;
	} else {
		return reflect(direction, normal);
	}
}

inline bool refract(const float3& incident, const float3& normal, float eta, float3& refracted) {
	float3 unit_incident = normalize(incident);
	float dt = dot(unit_incident, normal);
	float discriminant = 1.0f - eta * eta * (1.0f - dt * dt);
	if (discriminant > 0) {
		refracted = eta * (unit_incident - normal * dt) - normal * sqrt(discriminant);
		return true;
	}
	return false; // Total internal reflection
}


inline float2 fma(const  float2 a, const  float2 b, const float2 c) { return float2(fmaf(a.x, b.x, c.x), fmaf(a.y, b.y, c.y)); }
inline float3 fma(const  float3 a, const  float3 b, const float3 c) { return float3(fmaf(a.x, b.x, c.x), fmaf(a.y, b.y, c.y), fmaf(a.z, b.z, c.z)); }
inline float4 fma(const  float4 a, const  float4 b, const float4 c) { return float4(fmaf(a.x, b.x, c.x), fmaf(a.y, b.y, c.y), fmaf(a.z, b.z, c.z), fmaf(a.w, b.w, c.w)); }

inline float3 cross(const float3& a, const float3& b) { return make_float3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }

inline float3 uintToVec3(const uint& v) {
	float3 result;
	result.x = ((v >> 16) & 0xFF) / 255.0f;
	result.y = ((v >> 8) & 0xFF) / 255.0f;
	result.z = (v & 0xFF) / 255.0f;
	return result;
}

inline float fresnelSchlick(float cosTheta, float R0) {
	return R0 + (1.0f - R0) * static_cast<float>(pow(1.0f - cosTheta, 5));
}

inline float3 fresnelSchlick(float cosTheta, const float3& F0) {
	return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

inline float3 diffusereflection(const float3 N, uint& seed) {
	float3 R;
	do {
		R = make_float3(RandomFloat(seed) * 2 - 1, RandomFloat(seed) * 2 - 1, RandomFloat(seed) * 2 - 1);
	} while (dot(R, R) > 1);
	if (dot(R, N) < 0) R *= -1.0f;
	return normalize(R);
}

//diffuse reflection without using seed
inline float3 diffusereflection(const float3 N) {
	float3 R;
	do {
		R = make_float3(RandomFloat() * 2 - 1, RandomFloat() * 2 - 1, RandomFloat() * 2 - 1);
	} while (dot(R, R) > 1);
	if (dot(R, N) < 0) R *= -1.0f;
	return normalize(R);
}

inline static float3 RandomInUnitDisk() {
	float3 p;
	do {
		p = 2.0f * float3(RandomFloat(), RandomFloat(), 0) - float3(1, 1, 0);
	} while (dot(p, p) >= 1.0f);
	return p;
}

//taken from https://stackoverflow.com/questions/39245058/random-points-in-a-hexarea-shape/39262805#39262805
inline static float3 SampleHexagon() {
	// Algorithm based on https://stackoverflow.com/a/39262805/3841944 by user Severin Pappadeux
	// Coordinates are in the range [0, 1]
	const float width = 1;
	const float widthC = width / 2;
	const float widthA = (width - widthC) / 2;
	const float startC = widthA;
	const float startA = startC + widthC;
	const float slope = .5f / widthA;


	// 1. Remap x into the rectangle spanning C, A, B, b' and a'
	float x = startC + RandomFloat() * .75f;
	float y = RandomFloat();


	// 2. Move all points in a' and b' into a and b

	// If we are in the last third (A, b', B and a')
	if (startA < x) {
		float localX = x - startA;

		if (y > .5f) {  // And the upper half of it (in A or b')
			float localY = y - .5f;
			if (localY > .5f - localX * slope) { // And above the diagonal
				// Move the point so it is in the triangle to 'b'
				x -= startA;
				y -= .5f;
			}
		} else { // And the lower half of it (in B or a')
			float localY = y;
			if (localY < localX * slope) {  // And we are below the diagonal
				// Move the point so it is in the triangle to 'a'
				x -= startA;
				y += .5f;
			}
		}
	}

	// 3. Make to regular hexagon (currently the hexagon is too high, because we assumed width == height)

	// Divide the hexagon into 6 regular triangles and use the Pythagorean theorem, and this calculation should be fairly obvious
	float regularHexagonWidthToHeightRatio = sqrtf(1.f * 1.f - .5f * .5f);
	y = (y - .5f) * regularHexagonWidthToHeightRatio + .5f; // Center around 0, scale, center to .5

	// 4. Remap coordinates to -1 to 1 (optional)
	x = x * 2 - 1;
	y = y * 2 - 1;

	return float3(x, y, 0);
}

inline float3 SampleAmongUs() {
	// Define basic dimensions of the Among Us character
	const float bodyWidth = 0.5f; // Half-width of the character's body
	const float bodyHeight = 1.0f; // Total height of the character's body
	const float headRadius = 0.25f; // Radius for the head/visor area
	const float backpackWidth = 0.15f; // Width of the backpack

	// Generate a random point within a bounding box that encompasses the character
	float x = RandomFloat() * (bodyWidth + backpackWidth);
	float y = RandomFloat() * (bodyHeight + 2 * headRadius);

	// Adjust points to match the character's shape
	if (y < headRadius) { // If in the head area
		// Adjust for circular head shape
		float dx = x - bodyWidth;
		float dy = y - headRadius;
		if (dx * dx + dy * dy > headRadius * headRadius) {
			float angle = atan2(dy, dx);
			x = bodyWidth + cos(angle) * headRadius;
			y = headRadius + sin(angle) * headRadius;
		}
	} else if (y < bodyHeight) { // If in the body area
		// Adjust for body width, considering backpack
		if (x > bodyWidth) {
			x = bodyWidth + (x - bodyWidth) * (backpackWidth / (backpackWidth + bodyWidth - x));
		}
	} else { // If in the leg area
		// Split the lower part into two legs
		float legWidth = bodyWidth * 0.5f;
		if (x > legWidth) {
			// Push the point to the right leg
			x = legWidth + (x - legWidth) / 2;
		}
	}

	// Optionally, remap coordinates to center the character at the origin
	x = (x - (bodyWidth + backpackWidth) / 2) * 2;
	y = (y - (bodyHeight + headRadius) / 2) * 2;

	return float3(x, y, 0);
}


inline float3 cosineweighteddiffusereflection(const float3 N, const float r0, const float r1) {
	// based on Global Illumination Compendium
	float term1 = 6.28318531f * r0, term2 = sqrtf(1 - r1);
	float3 R(cosf(term1) * term2, sinf(term1) * term2, sqrtf(r1));
	float3 tmp = (fabs(N.x) > 0.99f) ? float3(0, 1, 0) : float3(1, 0, 0);
	float3 B = normalize(cross(N, tmp)), T = cross(B, N);
	return R.x * T + R.y * B + R.z * N;
}

inline float3 cosineweighteddiffusereflection(const float3 N, uint& seed) {
	// blog.demofox.org/2020/06/06/casual-shadertoy-path-tracing-2-image-improvement-and-glossy-reflections
	float3 R;
	do {
		R = make_float3(RandomFloat(seed) * 2 - 1, RandomFloat(seed) * 2 - 1, RandomFloat(seed) * 2 - 1);
	} while (dot(R, R) > 1);
	return normalize(N + normalize(R));
}

static const float PI = 3.14159265359f;
static const float TWO_PI = 6.28318530718f;
inline float3 RandomDirectionInHemisphere(const float3& normal) {
	float azimuth = TWO_PI * RandomFloat(); // RandomFloat() generates a random number between 0 and 1
	float inclination = acosf(1.0f - 2.0f * RandomFloat()); // Maps uniform distribution to cosine distribution

	// Convert spherical to Cartesian coordinates
	float sinf_inclination = sinf(inclination);

	float x = sinf_inclination * cosf(azimuth);
	float y = sinf_inclination * sinf(azimuth);
	float z = cosf(inclination);

	float3 direction = make_float3(x, y, z);

	// Ensure the direction is in the hemisphere defined by the normal
	if (dot(direction, normal) < 0.0f) {
		direction = -direction;
	}

	return direction;
}

inline float RandomFloatSigned() { return ((RandomUInt() * 2.3283064365387e-10f) - 0.5f) * 2.0f; }
inline float3 RandomOnHemiSphere(float3 normal) {
	float3 p = float3(RandomFloatSigned(), RandomFloatSigned(), RandomFloatSigned());

	return dot(normal, p) < 0 ? -p : p;
}

inline float probabilityToSampleDiffuse(float3 difColor, float3 specColor) {
	float lumDiffuse = dot(difColor, make_float3(0.2126f, 0.7152f, 0.0722f));
	float lumSpecular = dot(specColor, make_float3(0.2126f, 0.7152f, 0.0722f));
	float sum = lumDiffuse + lumSpecular;
	return lumDiffuse / sum;
}

inline float3 RandomDirectionInHemisphere(const float3& normal, uint& seed) {
	float r1 = RandomFloat(seed);
	float r2 = RandomFloat(seed);
	float r2s = sqrtf(r2);

	float3 w = normal;
	float3 u = (fabs(w.x) > 0.1f ? make_float3(0, 1, 0) : make_float3(1, 0, 0));
	u = normalize(cross(u, w));
	float3 v = cross(w, u);

	return normalize(u * cosf(2 * 3.14159265359f * r1) * r2s + v * sinf(2 * 3.14159265359f * r1) * r2s + w * sqrtf(1 - r2));
}

inline bool Refract(const float3& incident, const float3& normal, float ior, float3& refracted) {
	float cosi = clamp(-1.0f, 1.0f, dot(incident, normal));
	float etai = 1, etat = ior;
	float3 n = normal;
	if (cosi < 0) { cosi = -cosi; } // if the ray is inside the object, swap the indices and invert the normal to get the correct result
	else { std::swap(etai, etat); n = -normal; }
	float eta = etai / etat;
	float k = 1 - eta * eta * (1 - cosi * cosi);
	if (k < 0) {
		return false; // Total internal reflection
	} else {
		refracted = eta * incident + (eta * cosi - sqrtf(k)) * n;
		return true;
	}
}

inline float Fresnel(const float3& incident, const float3& normal, float ior) {
	float cosi = clamp(-1.0f, 1.0f, dot(incident, normal));
	float etai = 1, etat = ior;
	if (cosi > 0) { std::swap(etai, etat); }
	// Compute sini using Snell's law
	float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
	// Total internal reflection
	if (sint >= 1) {
		return 1.0f;
	} else {
		float cost = sqrtf(std::max(0.f, 1 - sint * sint));
		cosi = fabsf(cosi);
		float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
		float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
		return (Rs * Rs + Rp * Rp) / 2;
	}
}

inline double DistanceToOppositeFace(const float3& direction, float3& normal, const double voxelSize) {
	double cosTheta = dot(normal, direction);
	double sinAlpha = std::sqrt(1 - cosTheta * cosTheta);
	double distance = voxelSize / sinAlpha;
	return distance;
}

inline float3 GGXSampleNormal(const float3& N, float roughness, const float3& Xi) {
	// Convert the 2D random vector Xi into spherical coordinates
	float a = roughness * roughness;  // alpha^2
	float phi = 2.0f * PI * Xi.x;
	float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
	float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

	// Spherical to Cartesian coordinates - microfacet normal m
	float3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// Transform H from tangent space to world space
	float3 upVector = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 tangent = normalize(cross(upVector, N));
	float3 bitangent = cross(N, tangent);

	float3 m = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(m);
}

static const float INVPI = 0.31830988618f;

inline float D_GGX(const float NdotH, const float roughness) {
	float a2 = roughness * roughness;
	float d = ((NdotH * a2 - 1.0f) * NdotH + 1.0f);
	return a2 / (d * d * INVPI);
}

inline float G_Smith_Schlick_GGX(const float NdotV, const float roughness) {
	float a = roughness * roughness;
	float k = a / 2.0f;
	float G1 = max(NdotV, 0.001f) / (NdotV * (1.0f - k) + k);
	return G1;
}

inline float G_Smith(const float NdotV, const float NdotL, const float roughness) {
	return G_Smith_Schlick_GGX(NdotV, roughness) * G_Smith_Schlick_GGX(NdotL, roughness);
}

//inline float3 RandomPointOnPlane(const float3& normal, const float3& position, const float2 size) {
//	float3 tangent = normalize(cross(normal, make_float3(0, 0, 1)));
//	float3 bitangent = cross(normal, tangent);
//	float2 random = make_float2(RandomFloat(), RandomFloat());
//	return position + tangent * (random.x - 0.5f) * size.x + bitangent * (random.y - 0.5f) * size.y;
//}

// matrix classes
class mat2 {
public:
	mat2() = default;
	mat2(float2 a, float2 b) { cell[0] = a.x, cell[1] = b.x, cell[2] = a.y, cell[3] = b.y; }
	// mat2( float2 a, float2 b ) { cell[0] = a.x, cell[1] = a.y, cell[2] = b.x, cell[3] = b.y; }
	mat2(float a, float b, float c, float d) { cell[0] = a, cell[1] = b, cell[2] = c, cell[3] = d; }
	__declspec(align(16)) float cell[4] = { 1, 0, 0, 1 };
	constexpr static mat2 Identity() { return mat2{}; }
	float operator()(const int i, const int j) const { return cell[i * 2 + j]; }
	float& operator()(const int i, const int j) { return cell[i * 2 + j]; }
	float Determinant() const { return cell[0] * cell[3] - cell[1] * cell[2]; }
};

//mat3 class from Robin
class mat3 {
public:
	float cell[9];

	// Default constructor
	mat3() {
		// Initialize as identity matrix
		cell[0] = 1.0f; cell[1] = 0.0f; cell[2] = 0.0f;
		cell[3] = 0.0f; cell[4] = 1.0f; cell[5] = 0.0f;
		cell[6] = 0.0f; cell[7] = 0.0f; cell[8] = 1.0f;
	}

	// Constructor from three float3 vectors
	mat3(const float3& col1, const float3& col2, const float3& col3) {
		cell[0] = col1.x; cell[1] = col1.y; cell[2] = col1.z;
		cell[3] = col2.x; cell[4] = col2.y; cell[5] = col2.z;
		cell[6] = col3.x; cell[7] = col3.y; cell[8] = col3.z;
	}

	// Transpose method
	mat3 transpose() const {
		mat3 result;
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				result.cell[i * 3 + j] = cell[j * 3 + i];
			}
		}
		return result;
	}

	// Index operator for accessing elements
	float& operator()(const int i, const int j) {
		return cell[i * 3 + j];
	}

	// Addition operator
	mat3 operator+(const mat3& other) const {
		mat3 result;
		for (int i = 0; i < 9; ++i) {
			result.cell[i] = cell[i] + other.cell[i];
		}
		return result;
	}

	// Subtraction operator
	mat3 operator-(const mat3& other) const {
		mat3 result;
		for (int i = 0; i < 9; ++i) {
			result.cell[i] = cell[i] - other.cell[i];
		}
		return result;
	}

	// Equality operator
	bool operator==(const mat3& other) const {
		for (int i = 0; i < 9; ++i) {
			if (cell[i] != other.cell[i]) return false;
		}
		return true;
	}

	// Multiplication operator for mat3 * mat3
	mat3 operator*(const mat3& other) const {
		mat3 result;
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				result.cell[i * 3 + j] = 0.0f;
				for (int k = 0; k < 3; ++k) {
					result.cell[i * 3 + j] += cell[i * 3 + k] * other.cell[k * 3 + j];
				}
			}
		}
		return result;
	}

	// Multiplication operator for mat3 * float3
	float3 operator*(const float3& vec) const {
		float3 result;
		result.x = cell[0] * vec.x + cell[1] * vec.y + cell[2] * vec.z;
		result.y = cell[3] * vec.x + cell[4] * vec.y + cell[5] * vec.z;
		result.z = cell[6] * vec.x + cell[7] * vec.y + cell[8] * vec.z;
		return result;
	}
};

// 64 byte aligned 4x4 matrix class
class mat4 {
public:
	mat4() = default;
	mat4(float4 a, float4 b, float4 c, float4 d) {
		cell[0] = a.x, cell[1] = a.y, cell[2] = a.z, cell[3] = a.w;
		cell[4] = b.x, cell[5] = b.y, cell[6] = b.z, cell[7] = b.w;
		cell[8] = c.x, cell[9] = c.y, cell[10] = c.z, cell[11] = c.w;
		cell[12] = d.x, cell[13] = d.y, cell[14] = d.z, cell[15] = d.w;
	}
	mat4(float a, float b, float c, float d, float e, float f, float g, float h, float i, float j, float k, float l, float m, float n, float o, float p) {
		cell[0] = a, cell[1] = b, cell[2] = c, cell[3] = d;
		cell[4] = e, cell[5] = f, cell[6] = g, cell[7] = h;
		cell[8] = i, cell[9] = j, cell[10] = k, cell[11] = l;
		cell[12] = m, cell[13] = n, cell[14] = o, cell[15] = p;
	}
	__declspec(align(64)) float cell[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

	float& operator [] (const int idx) { return cell[idx]; }
	float operator()(const int i, const int j) const { return cell[i * 4 + j]; }
	float& operator()(const int i, const int j) { return cell[i * 4 + j]; }
	mat4& operator += (const mat4& a) {
		for (int i = 0; i < 16; i++) cell[i] += a.cell[i];
		return *this;
	}
	mat4& operator -= (const mat4& a) {
		for (int i = 0; i < 16; i++) cell[i] -= a.cell[i];
		return *this;
	}
	bool operator==(const mat4& m) {
		for (int i = 0; i < 16; i++) if (m.cell[i] != cell[i]) return false; return true;
	}
	float3 GetTranslation() const { return make_float3(cell[3], cell[7], cell[11]); }
	static mat4 FromColumnMajor(const mat4& T) {
		mat4 M;
		M.cell[0] = T.cell[0], M.cell[1] = T.cell[4], M.cell[2] = T.cell[8], M.cell[3] = T.cell[12];
		M.cell[4] = T.cell[1], M.cell[5] = T.cell[5], M.cell[6] = T.cell[9], M.cell[7] = T.cell[13];
		M.cell[8] = T.cell[2], M.cell[9] = T.cell[6], M.cell[10] = T.cell[10], M.cell[11] = T.cell[14];
		M.cell[12] = T.cell[3], M.cell[13] = T.cell[7], M.cell[14] = T.cell[11], M.cell[15] = T.cell[15];
		return M;
	}
	constexpr static mat4 Identity() { return mat4{}; }
	static mat4 ZeroMatrix() { mat4 r; memset(r.cell, 0, 64); return r; }
	static mat4 RotateX(const float a) { mat4 r; r.cell[5] = cosf(a); r.cell[6] = -sinf(a); r.cell[9] = sinf(a); r.cell[10] = cosf(a); return r; };
	static mat4 RotateY(const float a) { mat4 r; r.cell[0] = cosf(a); r.cell[2] = sinf(a); r.cell[8] = -sinf(a); r.cell[10] = cosf(a); return r; };
	static mat4 RotateZ(const float a) { mat4 r; r.cell[0] = cosf(a); r.cell[1] = -sinf(a); r.cell[4] = sinf(a); r.cell[5] = cosf(a); return r; };
	static mat4 Scale(const float s) { mat4 r; r.cell[0] = r.cell[5] = r.cell[10] = s; return r; }
	static mat4 Scale(const float3 s) { mat4 r; r.cell[0] = s.x, r.cell[5] = s.y, r.cell[10] = s.z; return r; }
	static mat4 Scale(const float4 s) { mat4 r; r.cell[0] = s.x, r.cell[5] = s.y, r.cell[10] = s.z, r.cell[15] = s.w; return r; }
	static mat4 Rotate(const float3& u, const float a) { return Rotate(u.x, u.y, u.z, a); }
	static mat4 Rotate(const float x, const float y, const float z, const float a) {
		const float c = cosf(a), l_c = 1 - c, s = sinf(a);
		// row major
		mat4 m;
		m[0] = x * x + (1 - x * x) * c, m[1] = x * y * l_c + z * s, m[2] = x * z * l_c - y * s, m[3] = 0;
		m[4] = x * y * l_c - z * s, m[5] = y * y + (1 - y * y) * c, m[6] = y * z * l_c + x * s, m[7] = 0;
		m[8] = x * z * l_c + y * s, m[9] = y * z * l_c - x * s, m[10] = z * z + (1 - z * z) * c, m[11] = 0;
		m[12] = m[13] = m[14] = 0, m[15] = 1;
		return m;
	}
	static mat4 LookAt(const float3 P, const float3 T) {
		const float3 z = normalize(T - P);
		const float3 x = normalize(cross(z, make_float3(0, 1, 0)));
		const float3 y = cross(x, z);
		mat4 M = Translate(P);
		M[0] = x.x, M[4] = x.y, M[8] = x.z;
		M[1] = y.x, M[5] = y.y, M[9] = y.z;
		M[2] = z.x, M[6] = z.y, M[10] = z.z;
		return M;
	}
	static mat4 LookAt(const float3& pos, const float3& look, const float3& up) {
		// PBRT's lookat
		mat4 cameraToWorld;
		// initialize fourth column of viewing matrix
		cameraToWorld(0, 3) = pos.x;
		cameraToWorld(1, 3) = pos.y;
		cameraToWorld(2, 3) = pos.z;
		cameraToWorld(3, 3) = 1;

		// initialize first three columns of viewing matrix
		float3 dir = normalize(look - pos);
		float3 right = cross(normalize(up), dir);
		if (dot(right, right) == 0) {
			printf(
				"\"up\" vector (%f, %f, %f) and viewing direction (%f, %f, %f) "
				"passed to LookAt are pointing in the same direction.  Using "
				"the identity transformation.\n",
				up.x, up.y, up.z, dir.x, dir.y, dir.z);
			return mat4();
		}
		right = normalize(right);
		float3 newUp = cross(dir, right);
		cameraToWorld(0, 0) = right.x, cameraToWorld(1, 0) = right.y;
		cameraToWorld(2, 0) = right.z, cameraToWorld(3, 0) = 0.;
		cameraToWorld(0, 1) = newUp.x, cameraToWorld(1, 1) = newUp.y;
		cameraToWorld(2, 1) = newUp.z, cameraToWorld(3, 1) = 0.;
		cameraToWorld(0, 2) = dir.x, cameraToWorld(1, 2) = dir.y;
		cameraToWorld(2, 2) = dir.z, cameraToWorld(3, 2) = 0.;
		return cameraToWorld.Inverted();
	}
	static mat4 Perspective(const float fovDegrees, const float aspect, const float n, const float f) {
		const float rad = fovDegrees * (PI / 180.0f);  // Convert fov from degrees to radians
		const float t = n * tanf(rad * 0.5f), r = t * aspect;
		mat4 M = mat4().Identity();  // Ensure this creates an identity matrix
		M[0] = n / r;
		M[5] = n / t;
		M[10] = -(f + n) / (f - n);
		M[14] = -2 * f * n / (f - n);  // Moved from M[11] to M[14]
		M[11] = 0;  // Ensure this is 0, which should be the case if M is initialized to identity
		M[15] = 0;
		return M;
	}
	static mat4 Translate(const float x, const float y, const float z) { mat4 r; r.cell[3] = x; r.cell[7] = y; r.cell[11] = z; return r; };
	static mat4 Translate(const float3 P) { mat4 r; r.cell[3] = P.x; r.cell[7] = P.y; r.cell[11] = P.z; return r; };

	//static mat4 Translate(const float x, const float y, const float z) {
	//	mat4 r;
	//	r.cell[12] = x;
	//	r.cell[13] = y;
	//	r.cell[14] = z;
	//	return r;
	//};
	//
	//static mat4 Translate(const float3 P) {
	//	mat4 r;
	//	r.cell[12] = P.x;
	//	r.cell[13] = P.y;
	//	r.cell[14] = P.z;
	//	return r;
	//};

	float Trace3() const { return cell[0] + cell[5] + cell[10]; }
#if 0
	//old broken code
	CHECK_RESULT mat4 Transposed() const {
		mat4 M;
		M[0] = cell[0], M[1] = cell[4], M[2] = cell[8];
		M[4] = cell[1], M[5] = cell[5], M[6] = cell[9];
		M[8] = cell[2], M[9] = cell[6], M[10] = cell[10];
		return M;
	}
#else
	//new fixed code
	CHECK_RESULT mat4 Transposed() const {
		mat4 M;
		M.cell[0] = cell[0];  M.cell[1] = cell[4];  M.cell[2] = cell[8];  M.cell[3] = cell[12];
		M.cell[4] = cell[1];  M.cell[5] = cell[5];  M.cell[6] = cell[9];  M.cell[7] = cell[13];
		M.cell[8] = cell[2];  M.cell[9] = cell[6];  M.cell[10] = cell[10]; M.cell[11] = cell[14];
		M.cell[12] = cell[3]; M.cell[13] = cell[7]; M.cell[14] = cell[11]; M.cell[15] = cell[15];
		return M;
	}
#endif

	CHECK_RESULT mat4 FastInvertedTransformNoScale() const {
		mat4 r;
#ifdef _MSC_VER
		// use SSE to transpose the 3x3 part
		__m128& inM0 = (__m128&)cell[0], & outM0 = (__m128&)r.cell[0];
		__m128& inM1 = (__m128&)cell[4], & outM1 = (__m128&)r.cell[4];
		__m128& inM2 = (__m128&)cell[8], & outM2 = (__m128&)r.cell[8];
		__m128 t0 = _mm_movelh_ps(inM0, inM1), t1 = _mm_movehl_ps(inM1, inM0);
		outM0 = _mm_shuffle_ps(t0, inM2, 0b11001000);
		outM1 = _mm_shuffle_ps(t0, inM2, 0b11011101);
		outM2 = _mm_shuffle_ps(t1, inM2, 0b11101000);
#else
		// fallback for crossplatform compatibility
		r[0] = cell[0], r[1] = cell[4], r[2] = cell[8];
		r[4] = cell[1], r[5] = cell[5], r[6] = cell[9];
		r[8] = cell[2], r[9] = cell[6], r[10] = cell[10];
#endif
		float3 T(cell[3], cell[7], cell[1]);
		r[3] = -(cell[3] * r[0] + cell[7] * r[1] + cell[11] * r[2]);
		r[7] = -(cell[3] * r[4] + cell[7] * r[5] + cell[11] * r[6]);
		r[11] = -(cell[3] * r[8] + cell[7] * r[9] + cell[11] * r[10]);
		return r;
	}
	CHECK_RESULT mat4 Inverted() const {
		// from MESA, via http://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
		const float inv[16] = {
			cell[5] * cell[10] * cell[15] - cell[5] * cell[11] * cell[14] - cell[9] * cell[6] * cell[15] +
			cell[9] * cell[7] * cell[14] + cell[13] * cell[6] * cell[11] - cell[13] * cell[7] * cell[10],
			-cell[1] * cell[10] * cell[15] + cell[1] * cell[11] * cell[14] + cell[9] * cell[2] * cell[15] -
			cell[9] * cell[3] * cell[14] - cell[13] * cell[2] * cell[11] + cell[13] * cell[3] * cell[10],
			cell[1] * cell[6] * cell[15] - cell[1] * cell[7] * cell[14] - cell[5] * cell[2] * cell[15] +
			cell[5] * cell[3] * cell[14] + cell[13] * cell[2] * cell[7] - cell[13] * cell[3] * cell[6],
			-cell[1] * cell[6] * cell[11] + cell[1] * cell[7] * cell[10] + cell[5] * cell[2] * cell[11] -
			cell[5] * cell[3] * cell[10] - cell[9] * cell[2] * cell[7] + cell[9] * cell[3] * cell[6],
			-cell[4] * cell[10] * cell[15] + cell[4] * cell[11] * cell[14] + cell[8] * cell[6] * cell[15] -
			cell[8] * cell[7] * cell[14] - cell[12] * cell[6] * cell[11] + cell[12] * cell[7] * cell[10],
			cell[0] * cell[10] * cell[15] - cell[0] * cell[11] * cell[14] - cell[8] * cell[2] * cell[15] +
			cell[8] * cell[3] * cell[14] + cell[12] * cell[2] * cell[11] - cell[12] * cell[3] * cell[10],
			-cell[0] * cell[6] * cell[15] + cell[0] * cell[7] * cell[14] + cell[4] * cell[2] * cell[15] -
			cell[4] * cell[3] * cell[14] - cell[12] * cell[2] * cell[7] + cell[12] * cell[3] * cell[6],
			cell[0] * cell[6] * cell[11] - cell[0] * cell[7] * cell[10] - cell[4] * cell[2] * cell[11] +
			cell[4] * cell[3] * cell[10] + cell[8] * cell[2] * cell[7] - cell[8] * cell[3] * cell[6],
			cell[4] * cell[9] * cell[15] - cell[4] * cell[11] * cell[13] - cell[8] * cell[5] * cell[15] +
			cell[8] * cell[7] * cell[13] + cell[12] * cell[5] * cell[11] - cell[12] * cell[7] * cell[9],
			-cell[0] * cell[9] * cell[15] + cell[0] * cell[11] * cell[13] + cell[8] * cell[1] * cell[15] -
			cell[8] * cell[3] * cell[13] - cell[12] * cell[1] * cell[11] + cell[12] * cell[3] * cell[9],
			cell[0] * cell[5] * cell[15] - cell[0] * cell[7] * cell[13] - cell[4] * cell[1] * cell[15] +
			cell[4] * cell[3] * cell[13] + cell[12] * cell[1] * cell[7] - cell[12] * cell[3] * cell[5],
			-cell[0] * cell[5] * cell[11] + cell[0] * cell[7] * cell[9] + cell[4] * cell[1] * cell[11] -
			cell[4] * cell[3] * cell[9] - cell[8] * cell[1] * cell[7] + cell[8] * cell[3] * cell[5],
			-cell[4] * cell[9] * cell[14] + cell[4] * cell[10] * cell[13] + cell[8] * cell[5] * cell[14] -
			cell[8] * cell[6] * cell[13] - cell[12] * cell[5] * cell[10] + cell[12] * cell[6] * cell[9],
			cell[0] * cell[9] * cell[14] - cell[0] * cell[10] * cell[13] - cell[8] * cell[1] * cell[14] +
			cell[8] * cell[2] * cell[13] + cell[12] * cell[1] * cell[10] - cell[12] * cell[2] * cell[9],
			-cell[0] * cell[5] * cell[14] + cell[0] * cell[6] * cell[13] + cell[4] * cell[1] * cell[14] -
			cell[4] * cell[2] * cell[13] - cell[12] * cell[1] * cell[6] + cell[12] * cell[2] * cell[5],
			cell[0] * cell[5] * cell[10] - cell[0] * cell[6] * cell[9] - cell[4] * cell[1] * cell[10] +
			cell[4] * cell[2] * cell[9] + cell[8] * cell[1] * cell[6] - cell[8] * cell[2] * cell[5]
		};
		const float det = cell[0] * inv[0] + cell[1] * inv[4] + cell[2] * inv[8] + cell[3] * inv[12];
		mat4 retVal;
		if (det != 0) {
			const float invdet = 1.0f / det;
			for (int i = 0; i < 16; i++) retVal.cell[i] = inv[i] * invdet;
		}
		return retVal;
	}

	CHECK_RESULT mat4 Inverted3x3() const {
		// via https://stackoverflow.com/questions/983999/simple-3x3-matrix-inverse-code-c
		const float invdet = 1.0f / (cell[0] * (cell[5] * cell[10] - cell[6] * cell[9]) -
			cell[4] * (cell[1] * cell[10] - cell[9] * cell[2]) +
			cell[8] * (cell[1] * cell[6] - cell[5] * cell[2]));
		mat4 R;
		R.cell[0] = (cell[5] * cell[10] - cell[6] * cell[9]) * invdet;
		R.cell[4] = (cell[8] * cell[6] - cell[4] * cell[10]) * invdet;
		R.cell[8] = (cell[4] * cell[9] - cell[8] * cell[5]) * invdet;
		R.cell[1] = (cell[9] * cell[2] - cell[1] * cell[10]) * invdet;
		R.cell[5] = (cell[0] * cell[10] - cell[8] * cell[2]) * invdet;
		R.cell[9] = (cell[1] * cell[8] - cell[0] * cell[9]) * invdet;
		R.cell[2] = (cell[1] * cell[6] - cell[2] * cell[5]) * invdet;
		R.cell[6] = (cell[2] * cell[4] - cell[0] * cell[6]) * invdet;
		R.cell[10] = (cell[0] * cell[5] - cell[1] * cell[4]) * invdet;
		return R;
	}

	inline float3 TransformVector(const float3& v) const {
#if USE_SIMD
		// Load the vector into an __m128
		__m128 vec = _mm_setr_ps(v.x, v.y, v.z, 0.0f);

		// Load the matrix rows into __m128
		__m128 row1 = _mm_loadu_ps(&cell[0]);
		__m128 row2 = _mm_loadu_ps(&cell[4]);
		__m128 row3 = _mm_loadu_ps(&cell[8]);

		// Duplicate each component of the vector for the dot product
		__m128 xxxx = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 yyyy = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 zzzz = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2, 2, 2, 2));

		// Perform the dot products
		__m128 rx = _mm_mul_ps(row1, xxxx);
		__m128 ry = _mm_mul_ps(row2, yyyy);
		__m128 rz = _mm_mul_ps(row3, zzzz);

		// Sum the results of each dot product to get the final transformed vector
		__m128 sum1 = _mm_add_ps(rx, ry);
		__m128 result = _mm_add_ps(sum1, rz);

		// Extract the results back into a float3
		float3 transformed;
		_mm_store_ss(&transformed.x, result);
		_mm_store_ss(&transformed.y, _mm_shuffle_ps(result, result, _MM_SHUFFLE(1, 1, 1, 1)));
		_mm_store_ss(&transformed.z, _mm_shuffle_ps(result, result, _MM_SHUFFLE(2, 2, 2, 2)));

		return transformed;
#else
		return make_float3(cell[0] * v.x + cell[1] * v.y + cell[2] * v.z,
			cell[4] * v.x + cell[5] * v.y + cell[6] * v.z,
			cell[8] * v.x + cell[9] * v.y + cell[10] * v.z);
#endif
	}

	inline float3 TransformPoint(const float3& v) const {
		// Fallback to the original
		const float3 res = make_float3(
			cell[0] * v.x + cell[1] * v.y + cell[2] * v.z + cell[3],
			cell[4] * v.x + cell[5] * v.y + cell[6] * v.z + cell[7],
			cell[8] * v.x + cell[9] * v.y + cell[10] * v.z + cell[11]);
		const float w = cell[12] * v.x + cell[13] * v.y + cell[14] * v.z + cell[15];
		if (w == 1) return res;
		return res * (1.f / w);
	}

	inline float3 TransformPointSIMD(const float3& v) const {
		// Load the point/vector into SIMD registers
		__m128 v_vec = _mm_set_ps(1.0f, v.z, v.y, v.x);

		// Load matrix rows into SIMD registers
		__m128 row0 = _mm_load_ps(&cell[0]);
		__m128 row1 = _mm_loadu_ps(&cell[4]);
		__m128 row2 = _mm_loadu_ps(&cell[8]);
		__m128 row3 = _mm_loadu_ps(&cell[12]);

		// Perform the dot product of matrix rows and point/vector
		__m128 res0 = _mm_dp_ps(row0, v_vec, 0xF1);
		__m128 res1 = _mm_dp_ps(row1, v_vec, 0xF2);
		__m128 res2 = _mm_dp_ps(row2, v_vec, 0xF4);
		__m128 res3 = _mm_dp_ps(row3, v_vec, 0xF8);

		// Combine the results
		__m128 res_combined = _mm_add_ps(_mm_add_ps(res0, res1), _mm_add_ps(res2, res3));

		// Extract the results
		float res_x = _mm_cvtss_f32(res_combined);
		float res_y = _mm_cvtss_f32(_mm_shuffle_ps(res_combined, res_combined, _MM_SHUFFLE(1, 1, 1, 1)));
		float res_z = _mm_cvtss_f32(_mm_shuffle_ps(res_combined, res_combined, _MM_SHUFFLE(2, 2, 2, 2)));
		float w = _mm_cvtss_f32(_mm_shuffle_ps(res_combined, res_combined, _MM_SHUFFLE(3, 3, 3, 3)));

		if (w != 1.0f) {
			res_x /= w;
			res_y /= w;
			res_z /= w;
		}

		return make_float3(res_x, res_y, res_z);
	}

	float3 GetScale() const {
		return make_float3(sqrtf(cell[0] * cell[0] + cell[4] * cell[4] + cell[8] * cell[8]),
			sqrtf(cell[1] * cell[1] + cell[5] * cell[5] + cell[9] * cell[9]),
			sqrtf(cell[2] * cell[2] + cell[6] * cell[6] + cell[10] * cell[10]));
	}

	inline float* Data() { return cell; }

	mat3 Get3x3() const {
		return mat3(make_float3(cell[0], cell[1], cell[2]), make_float3(cell[4], cell[5], cell[6]), make_float3(cell[8], cell[9], cell[10]));
	}

	void Print() const {
		printf("%f %f %f %f\n", cell[0], cell[1], cell[2], cell[3]);
		printf("%f %f %f %f\n", cell[4], cell[5], cell[6], cell[7]);
		printf("%f %f %f %f\n", cell[8], cell[9], cell[10], cell[11]);
		printf("%f %f %f %f\n", cell[12], cell[13], cell[14], cell[15]);
	}
};

mat4 operator * (const mat4& a, const mat4& b);
mat4 operator + (const mat4& a, const mat4& b);
mat4 operator * (const mat4& a, const float s);
mat4 operator * (const float s, const mat4& a);
bool operator == (const mat4& a, const mat4& b);
bool operator != (const mat4& a, const mat4& b);
float4 operator * (const mat4& a, const float4& b);
float4 operator * (const float4& a, const mat4& b);

bool operator == (const float4& a, const float4& b);
bool operator != (const float4& a, const float4& b);
bool operator >= (const float4& a, const float4& b);
bool operator <= (const float4& a, const float4& b);
bool operator > (const float4& a, const float4& b);
bool operator < (const float4& a, const float4& b);

inline mat2 operator+(const mat2& a, const mat2& b) { return mat2(a.cell[0] + b.cell[0], a.cell[1] + b.cell[1], a.cell[2] + b.cell[2], a.cell[3] + b.cell[3]); }
inline void operator+=(mat2& a, const mat2& b) { for (int i = 0; i < 4; i++) a.cell[i] += b.cell[i]; }
inline mat2 operator-(const mat2& a, const mat2& b) { return mat2(a.cell[0] - b.cell[0], a.cell[1] - b.cell[1], a.cell[2] - b.cell[2], a.cell[3] - b.cell[3]); }
inline void operator-=(mat2& a, const mat2& b) { for (int i = 0; i < 4; i++) a.cell[i] -= b.cell[i]; }

float3 TransformPosition(const float3& a, const mat4& M);
float3 TransformVector(const float3& a, const mat4& M);
float3 TransformPosition_SSE(const __m128& a, const mat4& M);
float3 TransformVector_SSE(const __m128& a, const mat4& M);

class quat // based on https://github.com/adafruit
{
public:
	quat() = default;
	quat(float _w, float _x, float _y, float _z) : w(_w), x(_x), y(_y), z(_z) {}
	quat(float _w, float3 v) : w(_w), x(v.x), y(v.y), z(v.z) {}
	float magnitude() const { return sqrtf(w * w + x * x + y * y + z * z); }
	void normalize() { float m = magnitude(); *this = this->scale(1 / m); }
	quat conjugate() const { return quat(w, -x, -y, -z); }
	void fromAxisAngle(const float3& axis, float theta) {
		w = cosf(theta / 2);
		const float s = sinf(theta / 2);
		x = axis.x * s, y = axis.y * s, z = axis.z * s;
	}
	void fromMatrix(const mat4& m) {
		float tr = m.Trace3(), S;
		if (tr > 0) {
			S = sqrtf(tr + 1.0f) * 2, w = 0.25f * S;
			x = (m(2, 1) - m(1, 2)) / S, y = (m(0, 2) - m(2, 0)) / S;
			z = (m(1, 0) - m(0, 1)) / S;
		} else if (m(0, 0) > m(1, 1) && m(0, 0) > m(2, 2)) {
			S = sqrt(1.0f + m(0, 0) - m(1, 1) - m(2, 2)) * 2;
			w = (m(2, 1) - m(1, 2)) / S, x = 0.25f * S;
			y = (m(0, 1) + m(1, 0)) / S, z = (m(0, 2) + m(2, 0)) / S;
		} else if (m(1, 1) > m(2, 2)) {
			S = sqrt(1.0f + m(1, 1) - m(0, 0) - m(2, 2)) * 2;
			w = (m(0, 2) - m(2, 0)) / S;
			x = (m(0, 1) + m(1, 0)) / S, y = 0.25f * S;
			z = (m(1, 2) + m(2, 1)) / S;
		} else {
			S = sqrt(1.0f + m(2, 2) - m(0, 0) - m(1, 1)) * 2;
			w = (m(1, 0) - m(0, 1)) / S, x = (m(0, 2) + m(2, 0)) / S;
			y = (m(1, 2) + m(2, 1)) / S, z = 0.25f * S;
		}
	}
	void toAxisAngle(float3& axis, float& angle) const {
		float s = sqrtf(1 - w * w);
		if (s == 0) return;
		angle = 2 * acosf(w);
		axis.x = x / s, axis.y = y / s, axis.z = z / s;
	}
	mat4 toMatrix() const {
		mat4 ret;
		ret.cell[0] = 1 - 2 * y * y - 2 * z * z;
		ret.cell[1] = 2 * x * y - 2 * w * z, ret.cell[2] = 2 * x * z + 2 * w * y, ret.cell[4] = 2 * x * y + 2 * w * z;
		ret.cell[5] = 1 - 2 * x * x - 2 * z * z;
		ret.cell[6] = 2 * y * z - 2 * w * x, ret.cell[8] = 2 * x * z - 2 * w * y, ret.cell[9] = 2 * y * z + 2 * w * x;
		ret.cell[10] = 1 - 2 * x * x - 2 * y * y;
		return ret;
	}
	float3 toEuler() const {
		float3 ret;
		float sqw = w * w, sqx = x * x, sqy = y * y, sqz = z * z;
		ret.x = atan2f(2.0f * (x * y + z * w), (sqx - sqy - sqz + sqw));
		ret.y = asinf(-2.0f * (x * z - y * w) / (sqx + sqy + sqz + sqw));
		ret.z = atan2f(2.0f * (y * z + x * w), (-sqx - sqy + sqz + sqw));
		return ret;
	}
	float3 toAngularVelocity(float dt) const {
		float3 ret;
		quat one(1, 0, 0, 0), delta = one - *this, r = (delta / dt);
		r = r * 2, r = r * one, ret.x = r.x, ret.y = r.y, ret.z = r.z;
		return ret;
	}
	float3 rotateVector(const float3& v) const {
		float3 qv = make_float3(x, y, z), t = cross(qv, v) * 2.0f;
		return v + t * w + cross(qv, t);
	}
	quat operator * (const quat& q) const {
		return quat(
			w * q.w - x * q.x - y * q.y - z * q.z, w * q.x + x * q.w + y * q.z - z * q.y,
			w * q.y - x * q.z + y * q.w + z * q.x, w * q.z + x * q.y - y * q.x + z * q.w
		);
	}
	static quat slerp(const quat& a, const quat& b, const float t) {
		// from https://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/index.htm
		quat qm;
		float cosHalfTheta = a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
		if (abs(cosHalfTheta) >= 1.0) {
			qm.w = a.w, qm.x = a.x, qm.y = a.y, qm.z = a.z;
			return qm;
		}
		float halfTheta = acosf(cosHalfTheta);
		float sinHalfTheta = sqrtf(1.0f - cosHalfTheta * cosHalfTheta);
		if (fabs(sinHalfTheta) < 0.001f) {
			qm.w = a.w * 0.5f + b.w * 0.5f, qm.x = a.x * 0.5f + b.x * 0.5f;
			qm.y = a.y * 0.5f + b.y * 0.5f, qm.z = a.z * 0.5f + b.z * 0.5f;
			return qm;
		}
		float ratioA = sinf((1 - t) * halfTheta) / sinHalfTheta;
		float ratioB = sinf(t * halfTheta) / sinHalfTheta;
		qm.w = (a.w * ratioA + b.w * ratioB), qm.x = (a.x * ratioA + b.x * ratioB);
		qm.y = (a.y * ratioA + b.y * ratioB), qm.z = (a.z * ratioA + b.z * ratioB);
		return qm;
	}
	quat operator + (const quat& q) const { return quat(w + q.w, x + q.x, y + q.y, z + q.z); }
	quat operator - (const quat& q) const { return quat(w - q.w, x - q.x, y - q.y, z - q.z); }
	quat operator / (float s) const { return quat(w / s, x / s, y / s, z / s); }
	quat operator * (float s) const { return scale(s); }
	quat scale(float s) const { return quat(w * s, x * s, y * s, z * s); }
	float w = 1, x = 0, y = 0, z = 0;
};

// axis aligned bounding box class
class aabb {
public:
	aabb() = default;
	aabb(__m128 a, __m128 b) { bmin4 = a, bmax4 = b; bmin[3] = bmax[3] = 0; }
	aabb(float3 a, float3 b) { bmin[0] = a.x, bmin[1] = a.y, bmin[2] = a.z, bmin[3] = 0, bmax[0] = b.x, bmax[1] = b.y, bmax[2] = b.z, bmax[3] = 0; }
	__inline void Reset() { bmin4 = _mm_set_ps1(1e34f), bmax4 = _mm_set_ps1(-1e34f); }
	bool Contains(const __m128& p) const {
		union { __m128 va4; float va[4]; };
		union { __m128 vb4; float vb[4]; };
		va4 = _mm_sub_ps(p, bmin4), vb4 = _mm_sub_ps(bmax4, p);
		return ((va[0] >= 0) && (va[1] >= 0) && (va[2] >= 0) &&
			(vb[0] >= 0) && (vb[1] >= 0) && (vb[2] >= 0));
	}
	__inline void Grow(const aabb& bb) { bmin4 = _mm_min_ps(bmin4, bb.bmin4); bmax4 = _mm_max_ps(bmax4, bb.bmax4); }
	__inline void Grow(const __m128& p) { bmin4 = _mm_min_ps(bmin4, p); bmax4 = _mm_max_ps(bmax4, p); }
	__inline void Grow(const __m128 min4, const __m128 max4) { bmin4 = _mm_min_ps(bmin4, min4); bmax4 = _mm_max_ps(bmax4, max4); }
	__inline void Grow(const float3& p) { __m128 p4 = _mm_setr_ps(p.x, p.y, p.z, 0); Grow(p4); }
	aabb Union(const aabb& bb) const { aabb r; r.bmin4 = _mm_min_ps(bmin4, bb.bmin4), r.bmax4 = _mm_max_ps(bmax4, bb.bmax4); return r; }
	static aabb Union(const aabb& a, const aabb& b) { aabb r; r.bmin4 = _mm_min_ps(a.bmin4, b.bmin4), r.bmax4 = _mm_max_ps(a.bmax4, b.bmax4); return r; }
	aabb Intersection(const aabb& bb) const { aabb r; r.bmin4 = _mm_max_ps(bmin4, bb.bmin4), r.bmax4 = _mm_min_ps(bmax4, bb.bmax4); return r; }
	__inline float Extend(const int axis) const { return bmax[axis] - bmin[axis]; }
	__inline float Minimum(const int axis) const { return bmin[axis]; }
	__inline float Maximum(const int axis) const { return bmax[axis]; }
	float Area() const {
		union { __m128 e4; float e[4]; };
		e4 = _mm_sub_ps(bmax4, bmin4);
		return max(0.0f, e[0] * e[1] + e[0] * e[2] + e[1] * e[2]);
	}
	int LongestAxis() const {
		int a = 0;
		if (Extend(1) > Extend(0)) a = 1;
		if (Extend(2) > Extend(a)) a = 2;
		return a;
	}
	// data members
#pragma warning ( push )
#pragma warning ( disable: 4201 /* nameless struct / union */ )
	union {
		struct {
			union { __m128 bmin4; float bmin[4]; struct { float3 bmin3; }; };
			union { __m128 bmax4; float bmax[4]; struct { float3 bmax3; }; };
		};
		__m128 bounds[2] = { _mm_setr_ps(1e34f, 1e34f, 1e34f, 0), _mm_setr_ps(-1e34f, -1e34f, -1e34f, 0) };
	};
#pragma warning ( pop )
	__inline void SetBounds(const __m128 min4, const __m128 max4) { bmin4 = min4; bmax4 = max4; }
	__inline __m128 Center() const { return _mm_mul_ps(_mm_add_ps(bmin4, bmax4), _mm_set_ps1(0.5f)); }
	__inline float Center(uint axis) const { return (bmin[axis] + bmax[axis]) * 0.5f; }
};

// matrix / vector multiplication
float3 TransformPosition(const float3& a, const mat4& M);
float3 TransformVector(const float3& a, const mat4& M);
float3 TransformPosition_SSE(const __m128& a, const mat4& M);
float3 TransformVector_SSE(const __m128& a, const mat4& M);

// Perlin noise
float noise2D(const float x, const float y);
float noise3D(const float x, const float y, const float z, float frequency = 5.0f, float amplitude = 0.5f);

inline quat EulerToQuaternion(const float3& euler) {
	float c1 = cosf(euler.x / 2);
	float c2 = cosf(euler.y / 2);
	float c3 = cosf(euler.z / 2);
	float s1 = sinf(euler.x / 2);
	float s2 = sinf(euler.y / 2);
	float s3 = sinf(euler.z / 2);
	quat q;
	q.x = c1 * c2 * s3 - s1 * s2 * c3;
	q.y = s1 * c2 * c3 + c1 * s2 * s3;
	q.z = c1 * s2 * c3 - s1 * c2 * s3;
	q.w = c1 * c2 * c3 + s1 * s2 * s3;
	return q;
}

inline float3 RandomPointOnPlane(const float3& position, const float3& rotation, const float2 size) {
	// Convert rotation from Euler angles to a quaternion
	quat rotationQuat = EulerToQuaternion(rotation);

	// Default plane normal pointing upwards before rotation
	float3 normal = make_float3(0, 1, 0);
	// Rotate the normal using the quaternion
	//normal = rotateVectorByQuaternion(normal, rotationQuat);
	normal = rotationQuat.rotateVector(normal);

	// Default tangent and bitangent before rotation
	float3 defaultTangent = make_float3(1, 0, 0); // X axis
	float3 defaultBitangent = make_float3(0, 0, 1); // Z axis

	// Rotate the tangent and bitangent using the quaternion
	/*float3 tangent = rotateVectorByQuaternion(defaultTangent, rotationQuat);*/
	float3 tangent = rotationQuat.rotateVector(defaultTangent);
	/*float3 bitangent = rotateVectorByQuaternion(defaultBitangent, rotationQuat);*/
	float3 bitangent = rotationQuat.rotateVector(defaultBitangent);

	// Generate random point on plane
	float2 random = make_float2(RandomFloat(), RandomFloat());
	return position + tangent * (random.x - 0.5f) * size.x + bitangent * (random.y - 0.5f) * size.y;
}