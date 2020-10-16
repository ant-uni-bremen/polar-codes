/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef LCG_H
#define LCG_H

#include <immintrin.h>
#include <cstdint>

#ifdef __AVX__
#include "avx_mathfun.h"
#endif


namespace SignalProcessing {
namespace Random {

template <class T>
class LCG;

/*!
 * Linear congruential pseudo-random number generator for unsigned 32-bit integers.
 */
template <>
class LCG<uint32_t>
{
public:
    LCG() : x(1) {}

    /*!
     * \brief Generate the next number of the pseudo-random sequence.
     * \return A pseudo-random number.
     */
    uint32_t operator()()
    {
        return x = x * 1664525 + 1013904223; // Numerical Recipes
    }

private:
    uint32_t x;
};

/*!
 * Linear congruential pseudo-random number generator for unsigned 64-bit integers.
 */
template <>
class LCG<uint64_t>
{
public:
    LCG() : x(1) {}

    /*!
     * \brief Generate the next number of the pseudo-random sequence.
     * \return A pseudo-random number.
     */
    uint64_t operator()() { return x = x * 2862933555777941757 + 3037000493; }

    /*!
     * \brief Initialize the PRNG.
     * \param s The seed to use.
     */
    void seed(uint64_t s) { x = s; }

private:
    uint64_t x;
};

/*!
 * Linear congruential pseudo-random number generator for single-precision numbers.
 */
template <>
class LCG<float>
{
public:
    LCG() : x(1) {}

    /*!
     * \brief Generate the next number of the pseudo-random sequence.
     * \return A pseudo-random number.
     */
    float operator()()
    {
        x = x * 1664525 + 1013904223; // Numerical Recipes
        union {
            uint32_t u;
            float f;
        } u;
        u.u = (x >> 9) | 0x3F800000;
        return u.f - 1.0f;
    }

private:
    uint32_t x;
};

/*!
 * Linear congruential pseudo-random number generator for double-precision numbers.
 */
template <>
class LCG<double>
{
public:
    LCG() : x(1) {}

    /*!
     * \brief Generate the next number of the pseudo-random sequence.
     * \return A pseudo-random number.
     */
    double operator()()
    {
        x = x * 2862933555777941757 +
            3037000493; // http://nuclear.llnl.gov/CNP/rng/rngman/node4.html
        union {
            uint64_t u;
            double f;
        } u;
        u.u = (x >> 12) | (uint64_t(0x3FF00000) << 32);
        return u.f - 1;
    }

private:
    uint64_t x;
};

////////////////////////////////////////////////////////////////////////////////
// AVX
#ifdef __AVX__

#ifndef __AVX2__
_PS256_CONST_TYPE(lcg_a, uint32_t, 1664525);
_PS256_CONST_TYPE(lcg_b, uint32_t, 1013904223);
_PS256_CONST_TYPE(lcg_mask, uint32_t, 0x3F800000);
AVX2_INTOP_USING_SSE2(mullo_epi32) // Actually uses SSE4.1 _mm_mullo_epi32()
AVX2_INTOP_USING_SSE2(or_si128)

template <>
class LCG<__m256>
{
public:
    LCG() : x(_mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8)) {}

    __m256 operator()()
    {
        x = _mm256_add_epi32_sse2(_mm256_mullo_epi32_sse2(x, *(__m256i*)_ps256_lcg_a),
                                  *(__m256i*)_ps256_lcg_b);
        __m256i u = _mm256_or_si128_sse2(_mm256_srli_epi32_sse2(x, 9),
                                         *(__m256i*)_ps256_lcg_mask);
        __m256 f = _mm256_sub_ps(_mm256_castsi256_ps(u), *(__m256*)_ps256_1);
        return f;
    }
    void seed(__m256i s) { x = s; }

private:
    __m256i x;
};

#else

_PS256_CONST_TYPE(lcg_a, uint32_t, 1664525);
_PS256_CONST_TYPE(lcg_b, uint32_t, 1013904223);
_PS256_CONST_TYPE(lcg_mask, uint32_t, 0x3F800000);

template <>
class LCG<__m256>
{
public:
    LCG() : x(_mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8)) {}

    __m256 operator()()
    {
        x = _mm256_add_epi32(_mm256_mullo_epi32(x, *(__m256i*)_ps256_lcg_a),
                             *(__m256i*)_ps256_lcg_b);
        __m256i u = _mm256_or_si256(_mm256_srli_epi32(x, 9), *(__m256i*)_ps256_lcg_mask);
        __m256 f = _mm256_sub_ps(_mm256_castsi256_ps(u), *(__m256*)_ps256_1);
        return f;
    }
    void seed(__m256i s) { x = s; }

private:
    __m256i x;
};

#endif // either AVX or AVX2
#else
#warning AVX-LCG not available
#endif // AVX

} // namespace Random
} // namespace SignalProcessing

#endif // LCG_H
