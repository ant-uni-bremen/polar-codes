/* -*- c++ -*- */
/*
 * Copyright 2018, 2020 Florian Lotze, Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/decoding/fastssc_avx_float.h>
#include <polarcode/encoding/butterfly_fip_packed.h>
#include <polarcode/polarcode.h>

#include <iostream>
#include <numeric>
#include <string>

#include <cmath>
#include <cstring> //for memset

#include <fmt/core.h>
#include <fmt/ranges.h>

namespace PolarCode {
namespace Decoding {

namespace FastSscAvx {

inline void memFloatFill(float* dst, float value, const size_t blockLength)
{
    if (blockLength >= 8) {
        const __m256 vec = _mm256_set1_ps(value);
        for (unsigned i = 0; i < blockLength; i += 8) {
            _mm256_store_ps(dst + i, vec);
        }
    } else {
        for (unsigned i = 0; i < blockLength; i++) {
            dst[i] = value;
        }
    }
}

Node::Node()
    : mBlockLength(0),
      xmDataPool(nullptr),
      mLlr(nullptr),
      mBit(nullptr),
      mInput(nullptr),
      mOutput(nullptr)
{
}

Node::Node(Node* other)
    : mBlockLength(other->mBlockLength),
      xmDataPool(other->xmDataPool),
      mLlr(xmDataPool->allocate(mBlockLength)),
      mBit(xmDataPool->allocate(mBlockLength)),
      mInput(other->mInput),
      mOutput(other->mOutput)
{
}

Node::Node(size_t blockLength, datapool_t* pool)
    : mBlockLength(blockLength),
      xmDataPool(pool),
      mLlr(pool->allocate(blockLength)),
      mBit(pool->allocate(blockLength)),
      mInput(mLlr->data),
      mOutput(mBit->data)
{
}

Node::~Node()
{
    xmDataPool->release(mLlr);
    xmDataPool->release(mBit);
}

void Node::decode() {}

void Node::setInput(float* input) { mInput = input; }

void Node::setOutput(float* output) { mOutput = output; }

unsigned Node::blockLength() { return mBlockLength; }

datapool_t* Node::pool() { return xmDataPool; }

float* Node::input() { return mInput; }

float* Node::output() { return mOutput; }


/*************
 * RateRNode
 * ***********/

RateRNode::RateRNode(const std::vector<unsigned>& frozenBits,
                     Node* parent,
                     ChildCreationFlags flags)
    : Node(parent)
{
    mBlockLength /= 2;

    std::vector<unsigned> leftFrozenBits, rightFrozenBits;
    splitFrozenBits(frozenBits, mBlockLength, leftFrozenBits, rightFrozenBits);

    if (flags & NO_LEFT) {
        mLeft = new Node();
    } else {
        // fmt::print("ctor: left  RateR( {} / {})\n", mBlockLength,
        // leftFrozenBits.size());
        mLeft = createDecoder(leftFrozenBits, this);
    }

    if (flags & NO_RIGHT) {
        mRight = new Node();
    } else {
        // fmt::print("ctor: right RateR( {} / {})\n", mBlockLength,
        // rightFrozenBits.size());
        mRight = createDecoder(rightFrozenBits, this);
    }

    mLeftLlr = xmDataPool->allocate(mBlockLength);
    mRightLlr = xmDataPool->allocate(mBlockLength);

    mLeft->setInput(mLeftLlr->data);
    mRight->setInput(mRightLlr->data);

    mLeft->setOutput(mOutput);
    mRight->setOutput(mOutput + mBlockLength);
}

RateRNode::~RateRNode()
{
    delete mLeft;
    delete mRight;
    xmDataPool->release(mLeftLlr);
    xmDataPool->release(mRightLlr);
}

void RateRNode::setOutput(float* output)
{
    mOutput = output;
    mLeft->setOutput(mOutput);
    mRight->setOutput(mOutput + mBlockLength);
}

void RateRNode::decode()
{
    F_function(mInput, mLeftLlr->data, mBlockLength);
    mLeft->decode();
    G_function(mInput, mRightLlr->data, mOutput, mBlockLength);
    mRight->decode();
    Combine(mOutput, mBlockLength);
}

/*************
 * ShortRateRNode
 * ***********/

ShortRateRNode::ShortRateRNode(const std::vector<unsigned>& frozenBits, Node* parent)
    : RateRNode(frozenBits, parent),
      mLeftBits(xmDataPool->allocate(8)),
      mRightBits(xmDataPool->allocate(8))
{
    mLeft->setOutput(mLeftBits->data);
    mRight->setOutput(mRightBits->data);
}

ShortRateRNode::~ShortRateRNode()
{
    xmDataPool->release(mLeftBits);
    xmDataPool->release(mRightBits);
}

void ShortRateRNode::setOutput(float* output) { mOutput = output; }

void ShortRateRNode::decode()
{
    F_function(mInput, mLeftLlr->data, mBlockLength);
    mLeft->decode();
    G_function(mInput, mRightLlr->data, mLeftBits->data, mBlockLength);
    mRight->decode();
    CombineBitsShort(mLeftBits->data, mRightBits->data, mOutput, mBlockLength);
}

/*************
 * ROneNode
 * ***********/

ROneNode::ROneNode(const std::vector<unsigned>& frozenBits, Node* parent)
    : RateRNode(frozenBits, parent, NO_RIGHT)
{
}

ROneNode::~ROneNode() {}

void ROneNode::decode()
{
    F_function(mInput, mLeftLlr->data, mBlockLength);
    mLeft->decode();
    rightDecode();
}

void ROneNode::rightDecode()
{
    for (unsigned i = 0; i < mBlockLength; i += 8) {
        __m256 Llr_l = _mm256_load_ps(mInput + i);
        __m256 Llr_r = _mm256_load_ps(mInput + mBlockLength + i);
        __m256 Bits = _mm256_load_ps(mOutput + i);
        __m256 HBits = hardDecode(Bits);

        __m256 Llr_o = _mm256_xor_ps(Llr_l, HBits);               // G-function
        Llr_o = _mm256_add_ps(Llr_o, Llr_r);                      // G-function
        /*nop*/                                                   // Rate 1 decoder
        _mm256_store_ps(mOutput + i, _mm256_xor_ps(Bits, Llr_o)); // Combine left bit
        _mm256_store_ps(mOutput + i + mBlockLength, Llr_o);       // Right bit
    }
}

/*************
 * ZeroRNode
 * ***********/

ZeroRNode::ZeroRNode(const std::vector<unsigned>& frozenBits, Node* parent)
    : RateRNode(frozenBits, parent, NO_LEFT)
{
}

ZeroRNode::~ZeroRNode() {}

void ZeroRNode::decode()
{
    G_function_0R(mInput, mRightLlr->data, mBlockLength);
    mRight->decode();
    Combine_0R(mOutput, mBlockLength);
}

/*************
 * RateZeroDecoder
 * ***********/

RateZeroDecoder::RateZeroDecoder(Node* parent) : Node(parent) {}

RateZeroDecoder::~RateZeroDecoder() {}

void RateZeroDecoder::decode() { memFloatFill(mOutput, INFINITY, mBlockLength); }

/*************
 * RateOneDecoder
 * ***********/

RateOneDecoder::RateOneDecoder(Node* parent) : Node(parent) {}

RateOneDecoder::~RateOneDecoder() {}

void RateOneDecoder::decode()
{
    for (unsigned i = 0; i < mBlockLength; i += 8) {
        __m256 llr = _mm256_load_ps(mInput + i);
        _mm256_store_ps(mOutput + i, llr);
    }
}

/*************
 * RepetitionDecoder
 * ***********/

RepetitionDecoder::RepetitionDecoder(Node* parent) : Node(parent) {}

RepetitionDecoder::~RepetitionDecoder() {}

void RepetitionDecoder::decode()
{
    __m256 LlrSum = _mm256_setzero_ps();

    RepetitionPrepare(mInput, mBlockLength);

    // Accumulate vectors
    for (unsigned i = 0; i < mBlockLength; i += 8) {
        LlrSum = _mm256_add_ps(LlrSum, _mm256_load_ps(mInput + i));
    }

    // Get final sum and save decoding result
    float Bits = reduce_add_ps(LlrSum);
    memFloatFill(mOutput, Bits, mBlockLength);
}

/*************
 * DoubleRepetitionDecoder
 * ***********/

DoubleRepetitionDecoder::DoubleRepetitionDecoder(Node* parent) : Node(parent)
{
    if (mBlockLength < 4) {
        throw std::invalid_argument(
            "Minimum block length for double Repetition code is 4!");
    }
}

DoubleRepetitionDecoder::~DoubleRepetitionDecoder() {}

void DoubleRepetitionDecoder::decode()
{
    __m256 llr_sum = _mm256_setzero_ps();

    RepetitionPrepare(mInput, mBlockLength);

    // Accumulate vectors
    for (unsigned i = 0; i < mBlockLength; i += 8) {
        llr_sum = _mm256_add_ps(llr_sum, _mm256_load_ps(mInput + i));
    }

    if (mBlockLength >= 8) {
        llr_sum = _mm256_add_ps(llr_sum, _mm256_permute2f128_ps(llr_sum, llr_sum, 1));

        llr_sum = _mm256_add_ps(llr_sum, _mm256_shuffle_ps(llr_sum, llr_sum, 0x4E));

        for (unsigned i = 0; i < mBlockLength; i += 8) {
            _mm256_store_ps(mOutput + i, llr_sum);
        }
    } else {
        float even_llr_sum = llr_sum[0] + llr_sum[2] + llr_sum[4] + llr_sum[6];

        float odd_llr_sum = llr_sum[1] + llr_sum[3] + llr_sum[5] + llr_sum[7];

        for (unsigned i = 0; i < mBlockLength; i += 2) {
            mOutput[i] = even_llr_sum;
            mOutput[i + 1] = odd_llr_sum;
        }
    }
}

/*************
 * SpcDecoder
 * ***********/

SpcDecoder::SpcDecoder(Node* parent) : Node(parent) {}

SpcDecoder::~SpcDecoder() {}

void SpcDecoder::decode()
{
    const __m256 sgnMask = _mm256_set1_ps(-0.0);
    __m256 parVec = _mm256_setzero_ps();
    unsigned minIdx = 0;
    float testAbs, minAbs = INFINITY;

    SpcPrepare(mInput, mBlockLength);

    for (unsigned i = 0; i < mBlockLength; i += 8) {
        __m256 vecIn = _mm256_load_ps(mInput + i);
        _mm256_store_ps(mOutput + i, vecIn);

        parVec = _mm256_xor_ps(parVec, vecIn);

        __m256 abs = _mm256_andnot_ps(sgnMask, vecIn);
        unsigned vecMin = _mm256_minidx_ps(abs, &testAbs);
        if (testAbs < minAbs) {
            minIdx = vecMin + i;
            minAbs = testAbs;
        }
    }

    // Flip least reliable bit, if neccessary
    union {
        float fParity;
        unsigned int iParity;
    };
    fParity = reduce_xor_ps(parVec);
    iParity &= 0x80000000;
    reinterpret_cast<unsigned int*>(mOutput)[minIdx] ^= iParity;
}


/*************
 * DoubleSpcDecoder
 * ***********/

void decode_double_spc(float* out, const float* in, const unsigned block_length)
{
    unsigned even_min_idx = 0;
    unsigned odd_min_idx = 0;
    bool even_parity = 0;
    bool odd_parity = 0;
    float even_min = std::numeric_limits<float>::max();
    float odd_min = std::numeric_limits<float>::max();

    for (unsigned i = 0; i < block_length; i += 2) {
        const float even = in[i];
        const float abs_even = std::abs(even);
        even_parity ^= std::signbit(even);
        if (abs_even < even_min) {
            even_min_idx = i;
            even_min = abs_even;
        }

        const float odd = in[i + 1];
        const float abs_odd = std::abs(odd);
        odd_parity ^= std::signbit(odd);
        if (abs_odd < odd_min) {
            odd_min_idx = i + 1;
            odd_min = abs_odd;
        }

        out[i] = even;
        out[i + 1] = odd;
    }

    // fmt::print("even idx={},\tval={},\tparity={}\n", even_min_idx, even_min,
    // even_parity); fmt::print("odd  idx={},\tval={},\tparity={}\n", odd_min_idx,
    // odd_min, odd_parity);

    reinterpret_cast<unsigned int*>(out)[even_min_idx] ^=
        (even_parity ? 0x80000000 : 0x00000000);
    reinterpret_cast<unsigned int*>(out)[odd_min_idx] ^=
        (odd_parity ? 0x80000000 : 0x00000000);
}


DoubleSpcDecoder::DoubleSpcDecoder(Node* parent) : Node(parent) {}

DoubleSpcDecoder::~DoubleSpcDecoder() {}

void DoubleSpcDecoder::decode()
{
    // SpcPrepare(mInput, mBlockLength);
    const float* llrs = mInput;

    __m256 parity = _mm256_setzero_ps();
    __m256 minvalues = _mm256_set1_ps(std::numeric_limits<float>::max());
    __m256 minindices = _mm256_setzero_ps();

    __m256 indices = _mm256_setr_ps(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);

    for (unsigned i = 0; i < mBlockLength; i += 8) {
        const __m256 part = _mm256_load_ps(llrs + i);
        _mm256_store_ps(mOutput + i, part);
        parity = _mm256_xor_ps(parity, part);
        minvalues = _mm256_argabsmin_ps(minindices, indices, minvalues, part);
        indices = _mm256_add_ps(indices, IDX_STEP);
    }

    const __m256 fourMin = _mm256_min4_ps(minvalues);
    const __m256 twoMin = _mm256_min2_ps(fourMin);

    const __m256 mask = _mm256_cmp_ps(minvalues, twoMin, _CMP_EQ_OQ);

    const unsigned even_idx_mask =
        __tzcnt_u32(_mm256_movemask_ps(_mm256_and_ps(mask, EVEN_MASK)));
    const unsigned odd_idx_mask =
        __tzcnt_u32(_mm256_movemask_ps(_mm256_and_ps(mask, ODD_MASK)));

    const unsigned even_idx = minindices[even_idx_mask];
    const unsigned odd_idx = minindices[odd_idx_mask];

    const __m256 parities = _mm256_reduce_xor_half_ps(parity);

    const bool even_parity = std::signbit(parities[0]);
    const bool odd_parity = std::signbit(parities[1]);

    reinterpret_cast<unsigned int*>(mOutput)[even_idx] ^=
        (even_parity ? 0x80000000 : 0x00000000);
    reinterpret_cast<unsigned int*>(mOutput)[odd_idx] ^=
        (odd_parity ? 0x80000000 : 0x00000000);
}


DoubleSpcDecoderShort8::DoubleSpcDecoderShort8(Node* parent) : Node(parent) {}

DoubleSpcDecoderShort8::~DoubleSpcDecoderShort8() {}

void DoubleSpcDecoderShort8::decode()
{
    const __m256 values = _mm256_load_ps(mInput);
    const __m256 minvalues = _mm256_abs_ps(values);

    const __m256 fourMin = _mm256_min4_ps(minvalues);
    const __m256 twoMin = _mm256_min2_ps(fourMin);
    const __m256 mask = _mm256_cmp_ps(minvalues, twoMin, _CMP_EQ_OQ);

    const __m256 signs = _mm256_and_ps(mask, SIGN_MASK);

    const __m256 parities = _mm256_reduce_xor_half_ps(values);
    const __m256 signed_mask = _mm256_and_ps(parities, signs);
    const __m256 result = _mm256_xor_ps(signed_mask, values);
    _mm256_store_ps(mOutput, result);
}


/*************
 * ZeroSpcDecoder
 * ***********/

ZeroSpcDecoder::ZeroSpcDecoder(Node* parent) : Node(parent)
{
    mTempBlock = xmDataPool->allocate(mBlockLength);
    mTempBlockPtr = mTempBlock->data;
}

ZeroSpcDecoder::~ZeroSpcDecoder() { xmDataPool->release(mTempBlock); }

void ZeroSpcDecoder::decode()
{
    const __m256 sgnMask = _mm256_set1_ps(-0.0);
    const size_t subBlockLength = mBlockLength / 2;
    __m256 parVec = _mm256_setzero_ps();
    unsigned minIdx = 0;
    float testAbs, minAbs = INFINITY;

    // Check parity equation
    for (unsigned i = 0; i < subBlockLength; i += 8) {
        // G-function with only frozen bits
        __m256 left = _mm256_load_ps(mInput + i);
        __m256 right = _mm256_load_ps(mInput + subBlockLength + i);
        __m256 llr = _mm256_add_ps(left, right);

        // Save output
        _mm256_store_ps(mOutput + i, right);
        _mm256_store_ps(mOutput + subBlockLength + i, right);

        // Update parity counter
        parVec = _mm256_xor_ps(parVec, llr);

        // Only search for minimum if there is a chance for smaller absolute value
        if (minAbs > 0) {
            __m256 abs = _mm256_andnot_ps(sgnMask, llr);
            unsigned vecMin = _mm256_minidx_ps(abs, &testAbs);
            if (testAbs < minAbs) {
                minIdx = vecMin + i;
                minAbs = testAbs;
            }
        }
    }

    // Flip least reliable bit, if neccessary
    union {
        float fParity;
        unsigned int iParity;
    };
    fParity = reduce_xor_ps(parVec);
    iParity &= 0x80000000;
    unsigned* iOutput = reinterpret_cast<unsigned*>(mOutput);
    iOutput[minIdx] ^= iParity;
    iOutput[minIdx + subBlockLength] ^= iParity;
}

/*************
 * ZeroSpcDecoderShort8
 * ***********/

ZeroSpcDecoderShort8::ZeroSpcDecoderShort8(Node* parent) : Node(parent) {}

ZeroSpcDecoderShort8::~ZeroSpcDecoderShort8() {}

void ZeroSpcDecoderShort8::decode()
{
    const __m256 input = _mm256_load_ps(mInput);
    const __m256 swaplane = _mm256_permute2f128_ps(input, input, 0b00000001);

    const __m256 spc_input = _mm256_add_ps(input, swaplane);
    const __m256 spc_output = _mm256_spc_right4_ps(spc_input);
    const __m256 result = _mm256_permute2f128_ps(spc_output, spc_output, 0b00010001);
    _mm256_store_ps(mOutput, result);
}


TripleRepetitionDecoder::TripleRepetitionDecoder(Node* parent) : Node(parent) {}

TripleRepetitionDecoder::~TripleRepetitionDecoder() {}

void TripleRepetitionDecoder::decode()
{
    __m256 input = _mm256_setzero_ps();
    for (unsigned i = 0; i < mBlockLength; i += 8) {
        const __m256 part = _mm256_load_ps(mInput + i);
        input = _mm256_add_ps(input, part);
    }

    const __m256 swaplane = _mm256_permute2f128_ps(input, input, 0b00000001);

    const __m256 spc_input = _mm256_add_ps(input, swaplane);
    const __m256 spc_output = _mm256_spc_right4_ps(spc_input);
    const __m256 result = _mm256_permute2f128_ps(spc_output, spc_output, 0b00010001);

    for (unsigned i = 0; i < mBlockLength; i += 8) {
        _mm256_store_ps(mOutput + i, result);
    }
}


namespace {

void calculate_repetition_generic(float* out,
                                  const float* in,
                                  const unsigned block_length)
{
    float sum = std::accumulate(in, in + block_length, 0.0f);
    std::fill(out, out + block_length, sum);
}

void calculate_spc_generic(float* out, const float* in, const unsigned block_length)
{
    std::copy(in, in + block_length, out);

    bool parity = false;
    unsigned idx = 0;
    float min = std::numeric_limits<float>::max();
    for (unsigned i = 0; i < block_length; i++) {
        parity ^= std::signbit(in[i]);
        float abs = std::abs(in[i]);

        if (min > abs) {
            min = abs;
            idx = i;
        }
    }
    if (parity) {
        out[idx] *= -1.0f;
    }
}

void calculate_f_generic(float* out,
                         const float* in_left,
                         const float* in_right,
                         const unsigned block_length)
{
    for (unsigned i = 0; i < block_length; ++i) {
        bool sign = std::signbit(in_left[i]) ^ std::signbit(in_right[i]);
        float min = std::min(std::abs(in_left[i]), std::abs(in_right[i]));
        out[i] = (sign ? -1.0f : 1.0f) * min;
    }
}

void calculate_g_generic(float* out,
                         const float* in_left,
                         const float* in_right,
                         const float* bits,
                         const unsigned block_length)
{
    for (unsigned i = 0; i < block_length; ++i) {
        out[i] = std::signbit(bits[i]) ? (in_right[i] - in_left[i])
                                       : (in_right[i] + in_left[i]);
    }
}

void combine(float* out,
             const float* in_left,
             const float* in_right,
             const unsigned block_length)
{
    for (unsigned i = 0; i < block_length; ++i) {
        bool sign = std::signbit(in_left[i]) ^ std::signbit(in_right[i]);
        out[i] = sign ? -1.0f : 1.0f;
    }
}

void decode_repspc_generic_8(float* out, const float* in)
{
    std::vector<float> repetition_input(4);
    calculate_f_generic(repetition_input.data(), in, in + 4, 4);

    // fmt::print("rep in: \t{}\n", repetition_input);

    std::vector<float> repetition_result(4);
    calculate_repetition_generic(repetition_result.data(), repetition_input.data(), 4);

    // fmt::print("rep out: \t{}\n", repetition_result);

    std::vector<float> spc_input(4);
    calculate_g_generic(spc_input.data(), in, in + 4, repetition_result.data(), 4);

    // fmt::print("spc in: \t{}\n", spc_input);

    std::vector<float> spc_output(4);
    calculate_spc_generic(spc_output.data(), spc_input.data(), 4);

    // fmt::print("spc out: \t{}\n", spc_output);

    combine(out, repetition_result.data(), spc_output.data(), 4);
    std::copy(spc_output.begin(), spc_output.end(), out + 4);
}

} // namespace

void decode_repone_generic_8(float* out, const float* in)
{
    std::vector<float> repetition_input(4);
    calculate_f_generic(repetition_input.data(), in, in + 4, 4);

    // fmt::print("rep in: \t{}\n", repetition_input);

    std::vector<float> repetition_result(4);
    calculate_repetition_generic(repetition_result.data(), repetition_input.data(), 4);

    // fmt::print("rep out: \t{}\n", repetition_result);

    std::vector<float> one_input(4);
    calculate_g_generic(one_input.data(), in, in + 4, repetition_result.data(), 4);

    // fmt::print("spc in: \t{}\n", spc_input);

    combine(out, repetition_result.data(), one_input.data(), 4);
    std::copy(one_input.begin(), one_input.end(), out + 4);
}

/*************
 * RepetitionRateOneDecoderShort8
 * ***********/

RepetitionRateOneDecoderShort8::RepetitionRateOneDecoderShort8(Node* parent)
    : Node(parent)
{
}

RepetitionRateOneDecoderShort8::~RepetitionRateOneDecoderShort8() {}

void RepetitionRateOneDecoderShort8::decode()
{
    const __m256 input = _mm256_load_ps(mInput);

    const __m256 swaplane = _mm256_permute2f128_ps(input, input, 0b00000001);

    const __m256 rep_in = _mm256_polarf_ps(input, swaplane);

    const __m256 reduce_half = _mm256_hadd_ps(rep_in, rep_in);
    const __m256 rep_result = _mm256_hadd_ps(reduce_half, reduce_half);

    const __m256 one_result = _mm256_polarg_ps(swaplane, input, rep_result);

    const __m256 combine_sign =
        _mm256_and_ps(SIGN_MASK, _mm256_xor_ps(rep_result, one_result));
    const __m256 broad_sign =
        _mm256_permute2f128_ps(combine_sign, combine_sign, 0b00010001);
    const __m256 sign_result = _mm256_xor_ps(broad_sign, _mm256_set1_ps(1.0f));
    const __m256 result = _mm256_blend_ps(sign_result, one_result, 0b11110000);

    _mm256_store_ps(mOutput, result);
}


void decode_type_five_generic(float* out, const float* in, const unsigned block_length)
{
    std::vector<float> llrs_vector(8, 0.0f);
    for (unsigned i = 0; i < block_length; i += 8) {
        for (unsigned j = 0; j < 8; ++j) {
            llrs_vector[j] += in[i + j];
        }
    }

    decode_repspc_generic_8(out, llrs_vector.data());

    for (unsigned i = 8; i < block_length; i += 8) {
        std::copy(out, out + 8, out + i);
    }
}

TypeFiveDecoder::TypeFiveDecoder(Node* parent) : Node(parent) {}

TypeFiveDecoder::~TypeFiveDecoder() {}

void TypeFiveDecoder::decode()
{
    __m256 llrs = _mm256_setzero_ps();

    for (unsigned i = 0; i < mBlockLength; i += 8) {
        const __m256 part = _mm256_load_ps(mInput + i);
        llrs = _mm256_add_ps(llrs, part);
    }

    const __m256 swaplane = _mm256_permute2f128_ps(llrs, llrs, 0b00000001);

    const __m256 rep_in = _mm256_polarf_ps(llrs, swaplane);

    const __m256 reduce_half = _mm256_hadd_ps(rep_in, rep_in);
    const __m256 rep_result = _mm256_hadd_ps(reduce_half, reduce_half);

    const __m256 spc_in = _mm256_polarg_ps(swaplane, llrs, rep_result);

    const __m256 spc_result = _mm256_spc_right4_ps(spc_in);

    const __m256 combine_sign =
        _mm256_and_ps(SIGN_MASK, _mm256_xor_ps(rep_result, spc_result));
    const __m256 broad_sign =
        _mm256_permute2f128_ps(combine_sign, combine_sign, 0b00010001);
    const __m256 sign_result = _mm256_xor_ps(broad_sign, _mm256_set1_ps(1.0f));
    const __m256 result = _mm256_blend_ps(sign_result, spc_result, 0b11110000);

    for (unsigned i = 0; i < mBlockLength; i += 8) {
        _mm256_storeu_ps(mOutput + i, result);
    }
}

// End of decoder definitions


Node* createDecoder(const std::vector<unsigned>& frozenBits, Node* parent)
{

    size_t blockLength = parent->blockLength();
    size_t frozenBitCount = frozenBits.size();

    // Begin with the two most simple codes:
    if (frozenBitCount == blockLength) {
        return new RateZeroDecoder(parent);
    }
    if (frozenBitCount == 0) {
        return new RateOneDecoder(parent);
    }

    // Following are "one bit unlike the others" codes:
    if (frozenBitCount == (blockLength - 1)) {
        return new RepetitionDecoder(parent);
    }
    if (frozenBitCount == 1) {
        return new SpcDecoder(parent);
    }

    // Following are "interleaved one bit unlike the others" codes:
    if (frozenBitCount == blockLength - 2) {
        for (unsigned i = 0; i < frozenBits.size(); i++) {
            if (frozenBits[i] != i) {
                throw std::invalid_argument(fmt::format("{}", frozenBits));
            }
        }
        return new DoubleRepetitionDecoder(parent);
    }

    if (frozenBitCount == 2 and frozenBits[0] == 0 and frozenBits[1] == 1) {
        if (blockLength == 8) {
            return new DoubleSpcDecoderShort8(parent);
        } else {
            return new DoubleSpcDecoder(parent);
        }
    }

    if (frozenBitCount == blockLength - 3 and blockLength > 8 and
        frozenBits[frozenBitCount - 1] == blockLength - 4) {
        for (unsigned i = 0; i < frozenBits.size(); i++) {
            if (frozenBits[i] != i) {
                throw std::invalid_argument(fmt::format("{}", frozenBits));
            }
        }
        return new TripleRepetitionDecoder(parent);
    }

    if (frozenBitCount == blockLength - 4 and
        frozenBits[frozenBitCount - 1] == blockLength - 4 and
        frozenBits[frozenBitCount - 2] == blockLength - 6) {
        return new TypeFiveDecoder(parent);
    }

    if (blockLength == 8 and frozenBitCount == 3 and frozenBits[0] == 0 and
        frozenBits[1] == 1 and frozenBits[2] == 2) {
        return new RepetitionRateOneDecoderShort8(parent);
    }

    if (blockLength == 8 and frozenBitCount == 5 and
        frozenBits[frozenBitCount - 1] == blockLength - 4 and
        frozenBits[frozenBitCount - 2] == blockLength - 5) {
        return new ZeroSpcDecoderShort8(parent);
    }

    if (blockLength == 8) {
        fmt::print("WARNING\t-->\tNO-opt 8bit decoder: N={}, notK={}, \t{}\n\t\tThis "
                   "should never happen!\n",
                   blockLength,
                   frozenBitCount,
                   frozenBits);
    }

    // Fallback: No special code available, split into smaller subcodes
    if (blockLength <= 8) {
        return new ShortRateRNode(frozenBits, parent);
    } else {
        std::vector<unsigned> leftFrozenBits, rightFrozenBits;
        splitFrozenBits(frozenBits, blockLength / 2, leftFrozenBits, rightFrozenBits);

        // Last case of optimization:
        // Common child node combination(s)
        if (leftFrozenBits.size() == blockLength / 2 && rightFrozenBits.size() == 1) {
            return new ZeroSpcDecoder(parent);
        }

        // Minor optimization:
        // Right rate-1
        if (rightFrozenBits.size() == 0) {
            return new ROneNode(frozenBits, parent);
        }
        // Left rate-0
        if (leftFrozenBits.size() == blockLength / 2) {
            return new ZeroRNode(frozenBits, parent);
        }
        return new RateRNode(frozenBits, parent);
    }
}

} // namespace FastSscAvx

FastSscAvxFloat::FastSscAvxFloat(size_t blockLength,
                                 const std::vector<unsigned>& frozenBits)
{
    initialize(blockLength, frozenBits);
}

FastSscAvxFloat::~FastSscAvxFloat() { clear(); }

void FastSscAvxFloat::clear()
{
    delete mEncoder;
    delete mRootNode;
    delete mNodeBase;
    delete mDataPool;
}

void FastSscAvxFloat::initialize(size_t blockLength,
                                 const std::vector<unsigned>& frozenBits)
{
    if (blockLength == mBlockLength && frozenBits == mFrozenBits) {
        return;
    }
    if (mBlockLength != 0) {
        clear();
    }
    mBlockLength = blockLength;
    // mFrozenBits = frozenBits;
    mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
    mEncoder = new Encoding::ButterflyFipPacked(mBlockLength, mFrozenBits);
    mEncoder->setSystematic(false);
    mDataPool = new DataPool<float, 32>();
    mNodeBase = new FastSscAvx::Node(mBlockLength, mDataPool);
    mRootNode = FastSscAvx::createDecoder(mFrozenBits, mNodeBase);
    mLlrContainer = new FloatContainer(mNodeBase->input(), mBlockLength);
    mBitContainer = new FloatContainer(mNodeBase->output(), mBlockLength);
    mLlrContainer->setFrozenBits(mFrozenBits);
    mBitContainer->setFrozenBits(mFrozenBits);
    mOutputContainer = new unsigned char[(mBlockLength - mFrozenBits.size() + 7) / 8];
}

bool FastSscAvxFloat::decode()
{
    mRootNode->decode();

    if (!mSystematic) {
        mEncoder->setFloatCodeword(dynamic_cast<FloatContainer*>(mBitContainer)->data());
        mEncoder->encode();
        mEncoder->getInformation(mOutputContainer);
    } else {
        mBitContainer->getPackedInformationBits(mOutputContainer);
    }

    bool result = mErrorDetector->check(mOutputContainer,
                                        (mBlockLength - mFrozenBits.size() + 7) / 8);
    return result;
}


} // namespace Decoding
} // namespace PolarCode
