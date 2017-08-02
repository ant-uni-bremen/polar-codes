#ifndef AVXCONVENIENCE_H
#define AVXCONVENIENCE_H

#include <immintrin.h>


/*
	AVX:    256 bit per register
	SSE:    128 bit per register
	float:   32 bit per value
*/
#define FLOATSPERVECTOR 8
#define BYTESPERVECTOR 32

class Avx2NotSupportedException{};

static inline float reduce_add_ps(__m256 x) {
    /* ( x3+x7, x2+x6, x1+x5, x0+x4 ) */
    const __m128 x128 = _mm_add_ps(_mm256_extractf128_ps(x, 1), _mm256_castps256_ps128(x));
    /* ( -, -, x1+x3+x5+x7, x0+x2+x4+x6 ) */
    const __m128 x64 = _mm_add_ps(x128, _mm_movehl_ps(x128, x128));
    /* ( -, -, -, x0+x1+x2+x3+x4+x5+x6+x7 ) */
    const __m128 x32 = _mm_add_ss(x64, _mm_shuffle_ps(x64, x64, 0x55));
    /* Conversion to float is a no-op on x86-64 */
    return _mm_cvtss_f32(x32);
}

static inline char reduce_adds_epi8(__m256i x) {
	const __m128i x128 = _mm_adds_epi8(_mm256_extracti128_si256(x,0), _mm256_extracti128_si256(x,1));
	const __m128i x64  = _mm_adds_epi8(x128, _mm_srli_si128(x128, 8));
	const __m128i x32  = _mm_adds_epi8(x64,  _mm_srli_si128(x64, 4));
	const __m128i x16  = _mm_adds_epi8(x32,  _mm_srli_si128(x32, 2));
	const __m128i x8   = _mm_adds_epi8(x16,  _mm_srli_si128(x16, 1));
	return ((char*)&x8)[0];
}

static inline float reduce_xor_ps(__m256 x) {
    /* ( x3+x7, x2+x6, x1+x5, x0+x4 ) */
    const __m128 x128 = _mm_xor_ps(_mm256_extractf128_ps(x, 1), _mm256_castps256_ps128(x));
    /* ( -, -, x1+x3+x5+x7, x0+x2+x4+x6 ) */
    const __m128 x64 = _mm_xor_ps(x128, _mm_movehl_ps(x128, x128));
    /* ( -, -, -, x0+x1+x2+x3+x4+x5+x6+x7 ) */
    const __m128 x32 = _mm_xor_ps(x64, _mm_shuffle_ps(x64, x64, 0x55));
    /* Conversion to float is a no-op on x86-64 */
    return _mm_cvtss_f32(x32);
}

static inline float _mm_reduce_xor_ps(__m128 x) {
    /* ( -, -, x1+x3+x5+x7, x0+x2+x4+x6 ) */
    const __m128 x64 = _mm_xor_ps(x, _mm_movehl_ps(x, x));
    /* ( -, -, -, x0+x1+x2+x3+x4+x5+x6+x7 ) */
    const __m128 x32 = _mm_xor_ps(x64, _mm_shuffle_ps(x64, x64, 0x55));
    /* Conversion to float is a no-op on x86-64 */
    return _mm_cvtss_f32(x32);
}

static inline char reduce_xor_si256(__m256i x) {
	const __m128i x128 = _mm_xor_si128(_mm256_extracti128_si256(x,0), _mm256_extracti128_si256(x,1));
	const __m128i x64  = _mm_xor_si128(x128, _mm_srli_si128(x128, 8));
	const __m128i x32  = _mm_xor_si128(x64,  _mm_srli_si128(x64, 4));
	const __m128i x16  = _mm_xor_si128(x32,  _mm_srli_si128(x32, 2));
	const __m128i x8   = _mm_xor_si128(x16,  _mm_srli_si128(x16, 1));
	return ((char*)&x8)[0];

}




static inline float _mm_reduce_add_ps(__m128 x) {
    /* ( -, -, x1+x3+x5+x7, x0+x2+x4+x6 ) */
    const __m128 x64 = _mm_add_ps(x, _mm_movehl_ps(x, x));
    /* ( -, -, -, x0+x1+x2+x3+x4+x5+x6+x7 ) */
    const __m128 x32 = _mm_add_ss(x64, _mm_shuffle_ps(x64, x64, 0x55));
    /* Conversion to float is a no-op on x86-64 */
    return _mm_cvtss_f32(x32);
}

static inline unsigned _mm_minidx_ps(__m128 x)
{
	const __m128 halfMinVec = _mm_min_ps(x, _mm_permute_ps(x, 0b01001110));
	const __m128 minVec = _mm_min_ps(halfMinVec, _mm_permute_ps(halfMinVec, 0b10110001));
	const __m128 mask = _mm_cmpeq_ps(x, minVec);
	return __tzcnt_u32(_mm_movemask_ps(mask));
}

static inline unsigned _mm256_minidx_ps(__m256 x, float *minVal) {
	//Lazy version

	const unsigned a = _mm_minidx_ps(_mm256_extractf128_ps(x, 0));
	const unsigned b = _mm_minidx_ps(_mm256_extractf128_ps(x, 1))+4;

	if(x[a] < x[b]) {
		*minVal = x[a];
		return a;
	}//else
		*minVal = x[b];
		return b;
}

/*!
 * \brief Expand 32 packed bits in _mask_ into 32 bytes.
 * \param mask Packed 32-bit integer
 * \return Vector, where bytes are set according to the respective bit in _mask_.
 */
static inline __m256i _mm256_get_mask_epi8(const unsigned int mask) {
  __m256i vmask(_mm256_set1_epi32(mask));
  const __m256i shuffle(_mm256_setr_epi64x(0x0000000000000000,
	  0x0101010101010101, 0x0202020202020202, 0x0303030303030303));
  vmask = _mm256_shuffle_epi8(vmask, shuffle);
  const __m256i bit_mask(_mm256_set1_epi64x(0x7fbfdfeff7fbfdfe));
  vmask = _mm256_or_si256(vmask, bit_mask);
  return _mm256_cmpeq_epi8(vmask, _mm256_set1_epi64x(-1));
}


/** \brief Returns the index of the smallest element of x.
 * 
 * This is an extension to the _mm_minpos_epu16()-function,
 * which is the only available function of its kind that returns the position
 * of the smallest unsigned 16-bit integer in a given vector.
 * _mm256_minpos_epu8() utilizes it to find the smallest unsigned 8-bit integer
 * in vector x and returns the respective position.
 * 
 */
unsigned _mm256_minpos_epu8(__m256i x, char *val = nullptr);

/*!
 * \brief Create a sub-vector-size child node by shifting the right-hand side bits.
 * \param x The vector containing left and right bits.
 * \param shift The number of bits to shift.
 * \return The right child node's bits.
 */
__m256i _mm256_subVectorShift_epu8(__m256i x, int shift);
__m256i _mm256_subVectorBackShift_epu8(__m256i x, int shift);

/*!
 * \brief Check if AVX2 is available
 */
bool featureCheckAvx2();

#endif //AVXCONVENIENCE

