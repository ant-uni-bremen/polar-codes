#ifndef PARAMETERS_H
#define PARAMETERS_H

#define CRCSIZE 8

#define EbN0_min    0.0
#define EbN0_max    6.0
#define EbN0_count 20


#define ConcurrentThreads 3

#define MinErrors      1000
#define MinIters       5000
#define MaxIters      10000

#define FLEXIBLE_DECODING

#ifdef __DEBUG__
//Can be commented out to suppress debugging output:
//#define RATEONEDEBUGOUTPUT
//#define DEBUGOUTPUT
#endif



/*
	IMPORTANT: Remember to recompile every(!) file that relies on this header!!!
*/






/*
	AVX512: 512 bit per register
	AVX:    256 bit per register
	float:   32 bit per value
*/

//#define CONFIG_AVX512

#ifdef CONFIG_AVX512

#include <immintrin.h>

#define FLOATSPERVECTOR 16
#define vec __m512

#define set1_ps _mm512_set1_ps
#define load_ps _mm512_load_ps
#define store_ps _mm512_store_ps

#define and_ps _mm512_and_ps
#define andnot_ps _mm512_andnot_ps
#define or_ps _mm512_or_ps
#define xor_ps _mm512_xor_ps

#define add_ps _mm512_add_ps
#define mul_ps _mm512_mul_ps

#define min_ps _mm512_min_ps

#else

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
#define mul_ps _mm256_mul_ps

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
#endif
