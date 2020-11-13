/* -*- c++ -*- */
/*
 * Copyright 2020 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <immintrin.h>
#include <limits>
#include <random>
#include <vector>
#include <algorithm>


std::vector<uint8_t> initialize_random_bit_vector(const unsigned size)
{
    std::random_device rnd_device;
    std::mt19937 mersenne_engine{ rnd_device() };
    std::uniform_int_distribution<uint8_t> dist{ 0, 255 };
    auto gen = [&dist, &mersenne_engine]() { return dist(mersenne_engine); };

    std::vector<uint8_t> vec(size);
    std::generate(begin(vec), end(vec), gen);
    return vec;
}

std::vector<float> initialize_random_vector(const unsigned size, const float variance = 10.0)
{
    std::random_device rnd_device;
    std::mt19937 mersenne_engine{ rnd_device() };
    std::normal_distribution<float> dist{ 0, variance };
    auto gen = [&dist, &mersenne_engine]() { return dist(mersenne_engine); };

    std::vector<float> vec(size);
    std::generate(begin(vec), end(vec), gen);
    return vec;
}

static const __m256 SIGN_MASK = _mm256_set1_ps(-0.0f);

inline __m256 _mm256_abs_ps(const __m256 values)
{
    return _mm256_andnot_ps(SIGN_MASK, values);
}

inline __m256 _mm256_cmplt_ps(const __m256 a, const __m256 b)
{
    return _mm256_cmp_ps(a, b, _CMP_GT_OQ);
}

inline float reduce_xor_ps(__m256 x)
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

template <unsigned size>
float calculate_parity(const float* llrs)
{
    __m256 parity = _mm256_setzero_ps();
    for (unsigned i = 0; i < size; i += 8) {
        __m256 part = _mm256_loadu_ps(llrs + i);
        parity = _mm256_xor_ps(parity, part);
    }
    return std::copysign(1.0f, reduce_xor_ps(parity));
}

__m256 _mm256_argabsmin_ps(__m256& minindices,
                           const __m256 indices,
                           const __m256 minvalues,
                           const __m256 values)
{
    const __m256 abs = _mm256_abs_ps(values);
    const __m256 mask = _mm256_cmplt_ps(abs, minvalues);
    minindices = _mm256_blendv_ps(indices, minindices, mask);
    return _mm256_blendv_ps(abs, minvalues, mask);
}

unsigned _mm256_argmin_ps(const __m256 x)
{
    const __m256 fourMin = _mm256_min_ps(x, _mm256_permute2f128_ps(x, x, 0b00000001));
    const __m256 twoMin = _mm256_min_ps(fourMin, _mm256_permute_ps(fourMin, 0b01001110));
    const __m256 oneMin = _mm256_min_ps(twoMin, _mm256_permute_ps(twoMin, 0b10110001));
    const __m256 mask = _mm256_cmp_ps(x, oneMin, _CMP_EQ_OQ);
    const int movmsk = _mm256_movemask_ps(mask);
    const unsigned idx = __tzcnt_u32(movmsk);
    return idx & 0x7;
}

template <unsigned size>
void calculate_spc(float* bits, const float* llrs)
{
    __m256 parity = _mm256_setzero_ps();
    __m256 minvalues = _mm256_set1_ps(std::numeric_limits<float>::max());
    __m256 minindices = _mm256_setzero_ps();
    __m256 indices = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0 };
    const __m256 step = _mm256_set1_ps(8.0);
    for (unsigned i = 0; i < size; i += 8) {
        __m256 part = _mm256_loadu_ps(llrs + i);
        _mm256_storeu_ps(bits + i, part);
        parity = _mm256_xor_ps(parity, part);
        minvalues = _mm256_argabsmin_ps(minindices, indices, minvalues, part);
        indices = _mm256_add_ps(indices, step);
    }

    const unsigned p = _mm256_argmin_ps(minvalues);
    const unsigned minidx = unsigned(minindices[p]);

    union {
        float fParity;
        unsigned int iParity;
    };
    fParity = reduce_xor_ps(parity);
    iParity &= 0x80000000;

    reinterpret_cast<unsigned int*>(bits)[minidx] ^= iParity;
}


template <unsigned size>
unsigned argabsmin_avx2(const float* values)
{
    __m256 minvalues = _mm256_set1_ps(std::numeric_limits<float>::max());
    __m256 minindices = _mm256_setzero_ps();
    __m256 indices = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0 };
    const __m256 step = _mm256_set1_ps(8.0);
    for (unsigned i = 0; i < size; i += 8) {
        __m256 part = _mm256_loadu_ps(values + i);
        minvalues = _mm256_argabsmin_ps(minindices, indices, minvalues, part);
        indices = _mm256_add_ps(indices, step);
    }
    const unsigned p = _mm256_argmin_ps(minvalues);
    return unsigned(minindices[p]);
}

template <unsigned size>
void calculate_spc_seq(float* bits, const float* llrs)
{
    union {
        float fParity;
        unsigned int iParity;
    };
    fParity = calculate_parity<size>(llrs);
    iParity &= 0x80000000;

    std::copy(llrs, llrs + size, bits);
    if (iParity) {
        const unsigned minidx = argabsmin_avx2<size>(llrs);

        reinterpret_cast<unsigned int*>(bits)[minidx] ^= iParity;
    }
}

template <unsigned size>
unsigned argabsmin(const float* values)
{
    float minval = std::numeric_limits<float>::max();
    unsigned minidx = 0;
    for (unsigned i = 0; i < size; ++i) {
        const float abs = std::abs(values[i]);
        if (abs < minval) {
            minidx = i;
            minval = abs;
        }
    }
    return minidx;
}

template <unsigned size>
unsigned calculate_bit_parity(const float* llrs)
{
    unsigned result = 0;
    for (unsigned i = 0; i < size; i++) {
        result ^= (llrs++ > 0) ? 0x1 : 0x0;
    }
    return result;
}
