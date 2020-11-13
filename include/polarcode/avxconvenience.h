/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef AVXCONVENIENCE_H
#define AVXCONVENIENCE_H

#include <immintrin.h>

union HybridFloat {
    float f;
    unsigned int u;
    int i;
};

#ifndef __AVX2__
#define BITSPERVECTOR 128
#define BYTESPERVECTOR 16
typedef __m128i fipv; // fixed point vector type

#define fi_load _mm_load_si128
#define fi_store _mm_store_si128

#define fi_setzero _mm_setzero_si128
#define fi_set1_epi8 _mm_set1_epi8

#define fi_blendv_epi8 _mm_blendv_epi8

#define fi_and _mm_and_si128
#define fi_or _mm_or_si128
#define fi_xor _mm_xor_si128

#define fi_add_epi64 _mm_add_epi64

#define fi_adds_epi8 _mm_adds_epi8
#define fi_subs_epi8 _mm_subs_epi8

#define fi_min_epi8 _mm_min_epi8
#define fi_min_epu8 _mm_min_epu8
#define fi_max_epi8 _mm_max_epi8
#define fi_max_epu8 _mm_max_epu8

#define fi_abs_epi8 _mm_abs_epi8
#define fi_sign_epi8 _mm_sign_epi8


#else
#define BITSPERVECTOR 256
#define BYTESPERVECTOR 32
typedef __m256i fipv; // fixed point vector type

#define fi_load _mm256_load_si256
#define fi_store _mm256_store_si256

#define fi_setzero _mm256_setzero_si256
#define fi_set1_epi8 _mm256_set1_epi8

#define fi_blendv_epi8 _mm256_blendv_epi8

#define fi_and _mm256_and_si256
#define fi_or _mm256_or_si256
#define fi_xor _mm256_xor_si256

#define fi_add_epi64 _mm256_add_epi64

#define fi_adds_epi8 _mm256_adds_epi8
#define fi_subs_epi8 _mm256_subs_epi8

#define fi_min_epi8 _mm256_min_epi8
#define fi_min_epu8 _mm256_min_epu8
#define fi_max_epi8 _mm256_max_epi8
#define fi_max_epu8 _mm256_max_epu8

#define fi_abs_epi8 _mm256_abs_epi8
#define fi_sign_epi8 _mm256_sign_epi8

#endif


/*
        AVX:    256 bit per register
        SSE:    128 bit per register
        float:   32 bit per value
*/
#define FLOATSPERVECTOR 8

#ifdef __AVX2__
static inline char reduce_adds_epi8(__m256i x)
{
    const __m128i x128 =
        _mm_adds_epi8(_mm256_extracti128_si256(x, 0), _mm256_extracti128_si256(x, 1));
    const __m128i x64 = _mm_adds_epi8(x128, _mm_srli_si128(x128, 8));
    const __m128i x32 = _mm_adds_epi8(x64, _mm_srli_si128(x64, 4));
    const __m128i x16 = _mm_adds_epi8(x32, _mm_srli_si128(x32, 2));
    const __m128i x8 = _mm_adds_epi8(x16, _mm_srli_si128(x16, 1));
    return ((char*)&x8)[0];
}

static inline int reduce_or_epi32(__m256i x)
{
    const __m128i x128 =
        _mm_or_si128(_mm256_extracti128_si256(x, 0), _mm256_extracti128_si256(x, 1));
    const __m128i x64 = _mm_or_si128(x128, _mm_srli_si128(x128, 8));
    const __m128i x32 = _mm_or_si128(x64, _mm_srli_si128(x64, 4));
    return _mm_cvtsi128_si32(x32);
}

static inline long long reduce_add_epi64(__m256i x)
{
    __m128i x128 =
        _mm_add_epi64(_mm256_extracti128_si256(x, 0), _mm256_extracti128_si256(x, 1));
    union {
        __m128i x64;
        long long i64[2];
    };
    x64 = _mm_add_epi64(x128, _mm_srli_si128(x128, 8));
    return i64[0];
}

static inline short reduce_adds_epi16(__m256i x)
{
    const __m128i x128 =
        _mm_adds_epi16(_mm256_extracti128_si256(x, 0), _mm256_extracti128_si256(x, 1));
    const __m128i x64 = _mm_adds_epi16(x128, _mm_srli_si128(x128, 8));
    const __m128i x32 = _mm_adds_epi16(x64, _mm_srli_si128(x64, 4));
    const __m128i x16 = _mm_adds_epi16(x32, _mm_srli_si128(x32, 2));
    return _mm_extract_epi16(x16, 0);
}

static inline unsigned char reduce_xor(__m256i x)
{
    const __m128i x128 =
        _mm_xor_si128(_mm256_extracti128_si256(x, 0), _mm256_extracti128_si256(x, 1));
    const __m128i x64 = _mm_xor_si128(x128, _mm_srli_si128(x128, 8));
    const __m128i x32 = _mm_xor_si128(x64, _mm_srli_si128(x64, 4));
    const __m128i x16 = _mm_xor_si128(x32, _mm_srli_si128(x32, 2));
    const __m128i x8 = _mm_xor_si128(x16, _mm_srli_si128(x16, 1));
    return (reinterpret_cast<const unsigned char*>(&x8))[0];
}

#endif

static inline float reduce_add_ps(__m256 x)
{
    /*	// ( x3+x7, x2+x6, x1+x5, x0+x4 )
            const __m128 x128 = _mm_add_ps(_mm256_extractf128_ps(x, 1),
       _mm256_castps256_ps128(x));
            // ( -, -, x1+x3+x5+x7, x0+x2+x4+x6 )
            const __m128 x64 = _mm_add_ps(x128, _mm_movehl_ps(x128, x128));
            // ( -, -, -, x0+x1+x2+x3+x4+x5+x6+x7 )
            const __m128 x32 = _mm_add_ss(x64, _mm_shuffle_ps(x64, x64, 0x55));
            // Conversion to float is a no-op on x86-64
            return _mm_cvtss_f32(x32);*/
    return x[0] + x[1] + x[2] + x[3] + x[4] + x[5] + x[6] + x[7];
    // __m256 first = _mm256_hadd_ps(x, _mm256_permute2f128_ps(x, x, 1));
    // first = _mm256_hadd_ps(first, first);
    // first = _mm256_hadd_ps(first, first);
    // return first[0];
}

#ifndef __AVX2__
static inline char reduce_adds_epi8(__m128i x)
{
    const __m128i x64 = _mm_adds_epi8(x, _mm_srli_si128(x, 8));
    const __m128i x32 = _mm_adds_epi8(x64, _mm_srli_si128(x64, 4));
    const __m128i x16 = _mm_adds_epi8(x32, _mm_srli_si128(x32, 2));
    const __m128i x8 = _mm_adds_epi8(x16, _mm_srli_si128(x16, 1));
    return ((char*)&x8)[0];
}

static inline int reduce_or_epi32(__m128i x)
{
    const __m128i x64 = _mm_or_si128(x, _mm_srli_si128(x, 8));
    const __m128i x32 = _mm_or_si128(x64, _mm_srli_si128(x64, 4));
    return _mm_cvtsi128_si32(x32);
}

static inline long long reduce_add_epi64(__m128i x)
{
    union {
        __m128i x64;
        long long i64[2];
    };
    x64 = _mm_add_epi64(x, _mm_srli_si128(x, 8));
    return i64[0];
}

static inline short reduce_adds_epi16(__m128i x)
{
    const __m128i x64 = _mm_adds_epi16(x, _mm_srli_si128(x, 8));
    const __m128i x32 = _mm_adds_epi16(x64, _mm_srli_si128(x64, 4));
    const __m128i x16 = _mm_adds_epi16(x32, _mm_srli_si128(x32, 2));
    return _mm_extract_epi16(x16, 0);
}
#endif

static inline float reduce_xor_ps(__m256 x)
{
    /* ( x3+x7, x2+x6, x1+x5, x0+x4 ) */
    const __m128 x128 =
        _mm_xor_ps(_mm256_extractf128_ps(x, 1), _mm256_castps256_ps128(x));
    /* ( -, -, x1+x3+x5+x7, x0+x2+x4+x6 ) */
    const __m128 x64 = _mm_xor_ps(x128, _mm_movehl_ps(x128, x128));
    /* ( -, -, -, x0+x1+x2+x3+x4+x5+x6+x7 ) */
    const __m128 x32 = _mm_xor_ps(x64, _mm_shuffle_ps(x64, x64, 0x55));
    /* Conversion to float is a no-op on x86-64 */
    return _mm_cvtss_f32(x32);
}

static inline float _mm_reduce_xor_ps(__m128 x)
{
    /* ( -, -, x1+x3+x5+x7, x0+x2+x4+x6 ) */
    const __m128 x64 = _mm_xor_ps(x, _mm_movehl_ps(x, x));
    /* ( -, -, -, x0+x1+x2+x3+x4+x5+x6+x7 ) */
    const __m128 x32 = _mm_xor_ps(x64, _mm_shuffle_ps(x64, x64, 0x55));
    /* Conversion to float is a no-op on x86-64 */
    return _mm_cvtss_f32(x32);
}

#ifndef __AVX2__
static inline unsigned char reduce_xor(__m128i x)
{
    const __m128i x64 = _mm_xor_si128(x, _mm_srli_si128(x, 8));
    const __m128i x32 = _mm_xor_si128(x64, _mm_srli_si128(x64, 4));
    const __m128i x16 = _mm_xor_si128(x32, _mm_srli_si128(x32, 2));
    const __m128i x8 = _mm_xor_si128(x16, _mm_srli_si128(x16, 1));
    return (reinterpret_cast<const unsigned char*>(&x8))[0];
}
#endif

static inline float _mm_reduce_add_ps(__m128 x)
{
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

static inline unsigned _mm256_minidx_ps(__m256 x, float* minVal)
{
    const __m256 fourMin = _mm256_min_ps(x, _mm256_permute2f128_ps(x, x, 0b00000001));
    const __m256 twoMin = _mm256_min_ps(fourMin, _mm256_permute_ps(fourMin, 0b01001110));
    const __m256 oneMin = _mm256_min_ps(twoMin, _mm256_permute_ps(twoMin, 0b10110001));
    const __m256 mask = _mm256_cmp_ps(x, oneMin, _CMP_EQ_OQ);
    const int movmsk = _mm256_movemask_ps(mask);
#ifdef __BMI__
    unsigned minIdx = __tzcnt_u32(movmsk);
#else
    unsigned minIdx = __builtin_ctz(movmsk);
#endif

    float* fx = reinterpret_cast<float*>(&x);

    *minVal = fx[minIdx];
    return minIdx;
}


#ifdef __AVX2__
/** \brief Returns the index of the smallest element of x.
 *
 * This is an extension to the _mm_minpos_epu16()-function,
 * which is the only available function of its kind that returns the position
 * of the smallest unsigned 16-bit integer in a given vector.
 * _mm_minpos_epu8() utilizes it to find the smallest unsigned 8-bit integer
 * in vector x and returns the respective position.
 *
 */
unsigned minpos_epu8(__m256i x, char* val = nullptr);

/*!
 * \brief Expand 32 packed bits in _mask_ into 32 bytes.
 * \param mask Packed 32-bit integer
 * \return Vector, where bytes are set according to the respective bit in _mask_.
 */
static inline __m256i _mm256_get_mask_epi8(const unsigned int mask)
{
    __m256i vmask(_mm256_set1_epi32(mask));
    const __m256i shuffle(_mm256_setr_epi64x(
        0x0000000000000000, 0x0101010101010101, 0x0202020202020202, 0x0303030303030303));
    vmask = _mm256_shuffle_epi8(vmask, shuffle);
    const __m256i bit_mask(_mm256_set1_epi64x(0x7fbfdfeff7fbfdfe));
    vmask = _mm256_or_si256(vmask, bit_mask);
    return _mm256_cmpeq_epi8(vmask, _mm256_set1_epi64x(-1));
}

__m256i subVectorShift_epu8(__m256i x, int shift);
__m256i subVectorBackShift_epu8(__m256i x, int shift);
__m256i subVectorShiftBytes_epu8(__m256i x, int shift);
__m256i subVectorBackShiftBytes_epu8(__m256i x, int shift);

#else

/** \brief Returns the index of the smallest element of x.
 *
 * This is an extension to the _mm_minpos_epu16()-function,
 * which is the only available function of its kind that returns the position
 * of the smallest unsigned 16-bit integer in a given vector.
 * _mm_minpos_epu8() utilizes it to find the smallest unsigned 8-bit integer
 * in vector x and returns the respective position.
 *
 */
unsigned minpos_epu8(__m128i x, char* val = nullptr);

static inline __m128i _mm_get_mask_epi8(const unsigned short mask)
{
    __m128i vmask(_mm_set1_epi32(mask));
    const __m128i shuffle(_mm_setr_epi64(_mm_setzero_si64(), _mm_set1_pi8(0x01)));
    vmask = _mm_shuffle_epi8(vmask, shuffle);
    const __m128i bit_mask(_mm_set_epi8(0x7f,
                                        0xbf,
                                        0xdf,
                                        0xef,
                                        0xf7,
                                        0xfb,
                                        0xfd,
                                        0xfe,
                                        0x7f,
                                        0xbf,
                                        0xdf,
                                        0xef,
                                        0xf7,
                                        0xfb,
                                        0xfd,
                                        0xfe));
    vmask = _mm_or_si128(vmask, bit_mask);
    return _mm_cmpeq_epi8(vmask, _mm_set1_epi8(-1));
}

/*!
 * \brief Create a sub-vector-size child node by shifting the right-hand side bits.
 * \param x The vector containing left and right bits.
 * \param shift The number of bits to shift.
 * \return The right child node's bits.
 */
__m128i subVectorShift_epu8(__m128i x, int shift);
__m128i subVectorBackShift_epu8(__m128i x, int shift);
__m128i subVectorShiftBytes_epu8(__m128i x, int shift);
__m128i subVectorBackShiftBytes_epu8(__m128i x, int shift);

#endif


__m256 _mm256_subVectorShift_ps(__m256 x, int shift);
__m256 _mm256_subVectorBackShift_ps(__m256 x, int shift);

inline static void memFloatFill(float* dst, float value, const size_t blockLength)
{
    if (blockLength < 8) {
        for (unsigned i = 0; i < blockLength; i++) {
            dst[i] = value;
        }
    } else {
        const __m256 vec = _mm256_set1_ps(value);
        for (unsigned i = 0; i < blockLength; i += 8) {
            _mm256_store_ps(dst + i, vec);
        }
    }
}


#endif // AVXCONVENIENCE
