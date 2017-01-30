#ifndef PARAMETERS_H
#define PARAMETERS_H

/* The following two defines can be used to change the behaviour
 * of the simulation.
 * 
 * ACCELERATED_MONTECARLO skips computations of nearly error-free SNRs
 * 
 * FLEXIBLE_DECODING does not use the specialized decoder implementation.
 * 					 Instead, the lookup tables are used to decide for the
 *					 next decoding operation.
 *
 * SYSTEMATIC_CODING utilizes systematic polar codes
 *
 */

//#define ACCELERATED_MONTECARLO
#define FLEXIBLE_DECODING
#define SYSTEMATIC_CODING
//#define ONLY_SCDECODING

/*
	IMPORTANT: Remember to recompile every(!) file that relies on this header!!!
*/






/*
	AVX:    256 bit per register
	float:   32 bit per value
*/

#include <immintrin.h>

#define FLOATSPERVECTOR 8
#define vec __m256

#define set1_ps _mm256_set1_ps
#define load_ps _mm256_load_ps
#define store_ps _mm256_store_ps

#define and_ps _mm256_and_ps
#define andnot_ps _mm256_andnot_ps
#define or_ps _mm256_or_ps
#define xor_ps _mm256_xor_ps

#define add_ps _mm256_add_ps
#define sub_ps _mm256_sub_ps
#define mul_ps _mm256_mul_ps

#define max_ps _mm256_max_ps
#define min_ps _mm256_min_ps

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

#endif

