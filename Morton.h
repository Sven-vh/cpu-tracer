#pragma once
#include <immintrin.h>
#include <cstdint>

using u64 = std::uint64_t;
using u32 = std::uint32_t;

/* 3D Morton BMI masks */
constexpr u64 BMI_3D_X_MASK = 0x9249249249249249;
constexpr u64 BMI_3D_Y_MASK = 0x2492492492492492;
constexpr u64 BMI_3D_Z_MASK = 0x4924924924924924;

//Credits to Max for these function
/* 3D coordinate to morton code. */
inline u64 morton_encode(const u32 x, const u32 y, const u32 z) {
	return _pdep_u64(x, BMI_3D_X_MASK) | _pdep_u64(y, BMI_3D_Y_MASK) | _pdep_u64(z, BMI_3D_Z_MASK);
}

/* Morton code to 3D coordinate. */
inline void morton_decode(const u64 m, u32& x, u32& y, u32& z) {
	x = (u32)(_pext_u64(m, BMI_3D_X_MASK));
	y = (u32)(_pext_u64(m, BMI_3D_Y_MASK));
	z = (u32)(_pext_u64(m, BMI_3D_Z_MASK));
}

inline void TransposePixelsAoS2SoA(__m128i* aos, __m128i* soa) {
    // Assume aos points to an array of 4 __m128i values, each containing the RGBA values of 4 pixels
    // soa is an output array of 4 __m128i values, where each will contain one component (R, G, B, or A) of the 16 pixels

    __m128i tmp0 = _mm_unpacklo_epi32(aos[0], aos[1]); // Interleave lower 2 pixels of the first two rows (R0G0R1G1 and B0A0B1A1)
    __m128i tmp1 = _mm_unpacklo_epi32(aos[2], aos[3]); // Same for the next two rows
    __m128i tmp2 = _mm_unpackhi_epi32(aos[0], aos[1]); // Interleave higher 2 pixels of the first two rows
    __m128i tmp3 = _mm_unpackhi_epi32(aos[2], aos[3]); // Same for the next two rows

    soa[0] = _mm_unpacklo_epi64(tmp0, tmp1); // R0G0R1G1R2G2R3G3
    soa[1] = _mm_unpackhi_epi64(tmp0, tmp1); // B0A0B1A1B2A2B3A3
    soa[2] = _mm_unpacklo_epi64(tmp2, tmp3); // Interleave tmp2 and tmp3 for the next component
    soa[3] = _mm_unpackhi_epi64(tmp2, tmp3); // Same for the alpha component
}

inline void TransposePixelsSoA2AoS(__m128i* soa, __m128i* aos) {
    // The inverse operation to repack the data back into AoS format after SIMD operations

    __m128i tmp0 = _mm_unpacklo_epi64(soa[0], soa[1]); // Interleave R and B components
    __m128i tmp1 = _mm_unpackhi_epi64(soa[0], soa[1]); // Interleave G and A components
    __m128i tmp2 = _mm_unpacklo_epi64(soa[2], soa[3]); // Do the same for the next set of components
    __m128i tmp3 = _mm_unpackhi_epi64(soa[2], soa[3]);

    aos[0] = _mm_unpacklo_epi32(tmp0, tmp1); // Pack the first row pixels
    aos[1] = _mm_unpackhi_epi32(tmp0, tmp1); // Pack the second row pixels
    aos[2] = _mm_unpacklo_epi32(tmp2, tmp3); // Pack the third row pixels
    aos[3] = _mm_unpackhi_epi32(tmp2, tmp3); // Pack the fourth row pixels
}