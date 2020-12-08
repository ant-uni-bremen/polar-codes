/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_DEC_AVX_FLOAT_H
#define PC_DEC_AVX_FLOAT_H

#include <polarcode/avxconvenience.h>
#include <polarcode/decoding/templatized_float.h>
#include <cassert>
#include <cmath>
#include <cstring>

namespace PolarCode {
namespace Decoding {
namespace FastSscAvx {

inline __m256 hardDecode(__m256 x)
{
    const __m256 mask = _mm256_set1_ps(-0.0f);
    return _mm256_and_ps(x, mask); // Get signs of LLRs
}

inline float hardDecode(float llr)
{
    /*	unsigned int* iLlr = reinterpret_cast<unsigned int*>(&llr);
            *iLlr &= 0x80000000;
            return llr;*/
    return (llr < 0) ? -0.0f : 0.0f;
}

static const __m256 IDX_STEP = _mm256_set1_ps(8.0);

static const __m256 EVEN_MASK =
    _mm256_cmp_ps(_mm256_setr_ps(1.0, -1.0, 2.0, -3.0, 4.0, -5.0, 6.0, -7.0),
                  _mm256_set1_ps(0.0),
                  _CMP_GT_OQ);

static const __m256 ODD_MASK =
    _mm256_cmp_ps(_mm256_setr_ps(-1.0, 1.0, -2.0, 3.0, -4.0, 5.0, -6.0, 7.0),
                  _mm256_set1_ps(0.0),
                  _CMP_GT_OQ);

static const __m256 SIGN_MASK = _mm256_set1_ps(-0.0f);

inline __m256 _mm256_abs_ps(const __m256 values)
{
    return _mm256_andnot_ps(SIGN_MASK, values);
}

inline __m256 _mm256_polarf_ps(const __m256 llrs0, const __m256 llrs1)
{
    __m256 sgnV = _mm256_and_ps(SIGN_MASK, _mm256_xor_ps(llrs0, llrs1));

    __m256 abs0 = _mm256_abs_ps(llrs0);
    __m256 abs1 = _mm256_abs_ps(llrs1);
    __m256 minV = _mm256_min_ps(abs0, abs1);
    return _mm256_or_ps(sgnV, minV);
}

inline void F_function_calc(__m256& Left, __m256& Right, float* Out)
{
    const __m256 result = _mm256_polarf_ps(Left, Right);
    _mm256_store_ps(Out, result);
}

inline __m256 _mm256_polarg_ps(const __m256 llrs0, const __m256 llrs1, const __m256 bits)
{
    // It would be great to scrap the next line. Require bits \in {-0.0, 0.0}!!
    const __m256 signbits = _mm256_and_ps(SIGN_MASK, bits);
    const __m256 signvalues = _mm256_xor_ps(signbits, llrs0);
    return _mm256_add_ps(signvalues, llrs1);
    // Benchmarks do not reveal any advantage for either yet.
    // const __m256 sum = _mm256_add_ps(llrs1, llrs0);
    // const __m256 dif = _mm256_sub_ps(llrs1, llrs0);
    // return _mm256_blendv_ps(sum, dif, bits);
}

inline void G_function_calc(__m256& Left, __m256& Right, __m256& Bits, float* Out)
{
    const __m256 result = _mm256_polarg_ps(Left, Right, Bits);
    _mm256_store_ps(Out, result);
}


template <unsigned subblock_length>
inline void calculate_polar_f(float* out_llrs, const float* in_llrs)
{
    for (unsigned i = 0; i < subblock_length; i += 8) {
        __m256 Left = _mm256_load_ps(in_llrs + i);
        __m256 Right = _mm256_load_ps(in_llrs + subblock_length + i);
        const __m256 result = _mm256_polarf_ps(Left, Right);
        _mm256_store_ps(out_llrs + i, result);
    }
}

inline void F_function(float* LLRin, float* LLRout, unsigned subBlockLength)
{
    __m256 Left, Right;
    if (subBlockLength < 8) {
        Left = _mm256_load_ps(LLRin);
        Right = _mm256_subVectorShift_ps(Left, subBlockLength);
        F_function_calc(Left, Right, LLRout);
    } else if (subBlockLength == 8) {
        calculate_polar_f<8>(LLRout, LLRin);
    } else if (subBlockLength == 16) {
        calculate_polar_f<16>(LLRout, LLRin);
    } else if (subBlockLength == 32) {
        calculate_polar_f<32>(LLRout, LLRin);
    } else if (subBlockLength == 64) {
        calculate_polar_f<64>(LLRout, LLRin);
    } else if (subBlockLength == 128) {
        calculate_polar_f<128>(LLRout, LLRin);
    } else if (subBlockLength == 256) {
        calculate_polar_f<256>(LLRout, LLRin);
    } else {
        for (unsigned i = 0; i < subBlockLength; i += 8) {
            Left = _mm256_load_ps(LLRin + i);
            Right = _mm256_load_ps(LLRin + subBlockLength + i);
            F_function_calc(Left, Right, LLRout + i);
        }
    }
}

inline void
boxplusVectors(float* inLlrA, float* inLlrB, float* llrOut, unsigned blockLength)
{
    __m256 va, vb;
    for (unsigned i = 0; i < blockLength; i += 8) {
        va = _mm256_load_ps(inLlrA + i);
        vb = _mm256_load_ps(inLlrB + i);
        F_function_calc(va, vb, llrOut + i);
    }
}

inline void boxplus(float* inLlrA, float* inLlrB, float* llrOut, unsigned blockLength)
{
    for (unsigned i = 0; i < blockLength; i++) {
        llrOut[i] = TemplatizedFloatCalc::F_function_calc(inLlrA[i], inLlrB[i]);
    }
}

inline void
G_function(float* LLRin, float* LLRout, float* BitsIn, unsigned subBlockLength)
{
    __m256 Left, Right, Bits;
    if (subBlockLength < 8) {
        Left = _mm256_load_ps(LLRin);
        Right = _mm256_subVectorShift_ps(Left, subBlockLength);
        Bits = _mm256_load_ps(BitsIn);
        G_function_calc(Left, Right, Bits, LLRout);
    } else {
        for (unsigned i = 0; i < subBlockLength; i += 8) {
            Left = _mm256_load_ps(LLRin + i);
            Right = _mm256_load_ps(LLRin + i + subBlockLength);
            Bits = _mm256_load_ps(BitsIn + i);
            G_function_calc(Left, Right, Bits, LLRout + i);
        }
    }
}

inline void G_function_0R(float* LLRin, float* LLRout, unsigned subBlockLength)
{
    __m256 Left, Right, Sum;
    for (unsigned i = 0; i < subBlockLength; i += 8) {
        Left = _mm256_load_ps(LLRin + i);
        Right = _mm256_load_ps(LLRin + i + subBlockLength);
        Sum = _mm256_add_ps(Left, Right);
        _mm256_store_ps(LLRout + i, Sum);
    }
}


inline void PrepareForShortOperation(__m256& Left, const unsigned subBlockLength)
{
    for (unsigned int i = subBlockLength; i < FLOATSPERVECTOR; ++i) {
        Left[i] = 0.0f;
    }
}

inline void MoveRightBits(__m256& Right, const unsigned subBlockLength)
{
    Right = _mm256_subVectorBackShift_ps(Right, subBlockLength);
}

inline void Combine(float* Bits, const unsigned bitCount)
{
    for (unsigned i = 0; i < bitCount; i += 8) {
        __m256 tempL = _mm256_load_ps(Bits + i);
        __m256 tempR = _mm256_load_ps(Bits + i + bitCount);
        _mm256_store_ps(Bits + i, _mm256_xor_ps(tempL, tempR));
    }
}

inline void Combine_0R(float* Bits, const unsigned bitCount)
{
    for (unsigned i = 0; i < bitCount; ++i) {
        Bits[i] = Bits[i + bitCount];
    }
}

inline void
CombineBitsShort(float* Left, float* Right, float* Out, const unsigned subBlockLength)
{
    __m256 LeftV = _mm256_load_ps(Left);
    __m256 RightV = _mm256_load_ps(Right);
    __m256 OutV;

    PrepareForShortOperation(LeftV, subBlockLength);
    PrepareForShortOperation(RightV, subBlockLength);

    OutV = _mm256_xor_ps(LeftV, RightV);

    // Copy operation for lower bits
    MoveRightBits(RightV, subBlockLength);
    OutV = _mm256_or_ps(RightV, OutV);
    _mm256_store_ps(Out, OutV);
}

inline void
CombineBitsLong(float* Left, float* Right, float* Out, const unsigned subBlockLength)
{
    __m256 LeftV;
    __m256 RightV;
    for (unsigned i = 0; i < subBlockLength; i += 8) {
        LeftV = _mm256_load_ps(Left + i);
        RightV = _mm256_load_ps(Right + i);

        _mm256_store_ps(Out + i, _mm256_xor_ps(LeftV, RightV));
        _mm256_store_ps(Out + i + subBlockLength, RightV);
    }
}

inline void RepetitionPrepare(float* x, const unsigned codeLength)
{
    for (unsigned i = codeLength; i < 8; ++i) {
        x[i] = 0.0f;
    }
}

inline void SpcPrepare(float* x, const unsigned codeLength)
{
    for (unsigned i = codeLength; i < 8; ++i) {
        x[i] = INFINITY;
    }
}

inline __m256 _mm256_cmplt_ps(const __m256 a, const __m256 b)
{
    return _mm256_cmp_ps(a, b, _CMP_GT_OQ);
}

inline __m256 _mm256_argabsmin_ps(__m256& minindices,
                                  const __m256 indices,
                                  const __m256 minvalues,
                                  const __m256 values)
{
    const __m256 abs = _mm256_abs_ps(values);
    const __m256 mask = _mm256_cmplt_ps(abs, minvalues);
    minindices = _mm256_blendv_ps(indices, minindices, mask);
    return _mm256_blendv_ps(abs, minvalues, mask);
}

inline __m256 _mm256_min4_ps(const __m256 x)
{
    return _mm256_min_ps(x, _mm256_permute2f128_ps(x, x, 0b00000001));
}

inline __m256 _mm256_min2_ps(const __m256 fourMin)
{
    return _mm256_min_ps(fourMin, _mm256_permute_ps(fourMin, 0b01001110));
}

inline unsigned _mm256_argmin_ps(const __m256 x)
{
    const __m256 fourMin = _mm256_min4_ps(x);
    const __m256 twoMin = _mm256_min2_ps(fourMin);
    const __m256 oneMin = _mm256_min_ps(twoMin, _mm256_permute_ps(twoMin, 0b10110001));
    const __m256 mask = _mm256_cmp_ps(x, oneMin, _CMP_EQ_OQ);
    const int movmsk = _mm256_movemask_ps(mask);
    const unsigned idx = __tzcnt_u32(movmsk);
    return idx & 0x7;
}

inline __m256 _mm256_spc_right4_ps(const __m256 spc_in)
{
    const __m256 abs = _mm256_abs_ps(spc_in);
    const __m256 twoMin = _mm256_min2_ps(abs);
    const __m256 oneMin = _mm256_min_ps(twoMin, _mm256_permute_ps(twoMin, 0b10110001));
    const __m256 mask = _mm256_cmp_ps(abs, oneMin, _CMP_EQ_OQ);

    const __m256 permute_half = _mm256_permute_ps(spc_in, 0b01001110);
    const __m256 xor_half_h = _mm256_xor_ps(spc_in, permute_half);
    const __m256 xor_half = _mm256_and_ps(SIGN_MASK, xor_half_h);
    const __m256 permute_full = _mm256_permute_ps(xor_half, 0b10110001);
    const __m256 xor_full = _mm256_xor_ps(xor_half, permute_full);
    return _mm256_xor_ps(spc_in, _mm256_and_ps(xor_full, mask));
}

} // namespace FastSscAvx

/*!
 * \brief Convert block length to minimum AVX-vector count.
 * \param blockLength Bits to store
 * \return The number of AVX-vectors required to store _blockLength_ char bits.
 */
inline size_t nBit2fvecCount(size_t blockLength) { return (blockLength + 7) / 8; }
/*!
 * \brief Expand the blocklength to AVX-vector boundaries of eight floats.
 *
 * For correct AVX-vector access, groups of eight floats need to be allocated.
 * This function expands the given block length to a multiple of eight.
 *
 * \param blockLength Bits to store
 * \return The expanded block length.
 */
inline size_t nBit2fCount(size_t blockLength) { return (blockLength + 7) & (~7); }


} // namespace Decoding
} // namespace PolarCode

#endif
