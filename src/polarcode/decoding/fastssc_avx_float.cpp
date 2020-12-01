/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/decoding/fastssc_avx_float.h>
#include <polarcode/encoding/butterfly_fip_packed.h>
#include <polarcode/polarcode.h>

#include <iostream>
#include <string>

#include <cmath>
#include <cstring> //for memset

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
        mLeft = createDecoder(leftFrozenBits, this);
    }

    if (flags & NO_RIGHT) {
        mRight = new Node();
    } else {
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

SpcDecoder::SpcDecoder(Node* parent) : Node(parent)
{
    mTempBlock = xmDataPool->allocate(mBlockLength);
    mTempBlockPtr = mTempBlock->data;
}

SpcDecoder::~SpcDecoder() { xmDataPool->release(mTempBlock); }

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

DoubleSpcDecoder::DoubleSpcDecoder(Node* parent) : Node(parent)
{
    mTempBlock = xmDataPool->allocate(mBlockLength);
    mTempBlockPtr = mTempBlock->data;
}

DoubleSpcDecoder::~DoubleSpcDecoder() { xmDataPool->release(mTempBlock); }

void DoubleSpcDecoder::decode()
{
    SpcPrepare(mInput, mBlockLength);
    const float* llrs = mInput;
    __m256 parity = _mm256_setzero_ps();
    __m256 minvalues = _mm256_set1_ps(std::numeric_limits<float>::max());
    __m256 minindices = _mm256_setzero_ps();
    __m256 indices = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0 };
    const __m256 step = _mm256_set1_ps(8.0);
    for (unsigned i = 0; i < mBlockLength; i += 8) {
        __m256 part = _mm256_loadu_ps(llrs + i);
        _mm256_storeu_ps(mOutput + i, part);
        parity = _mm256_xor_ps(parity, part);
        minvalues = _mm256_argabsmin_ps(minindices, indices, minvalues, part);
        indices = _mm256_add_ps(indices, step);
    }

    const __m256 fourMin = _mm256_min4_ps(minvalues);
    const __m256 twoMin = _mm256_min2_ps(fourMin);
    const __m256 mask = _mm256_cmp_ps(minvalues, twoMin, _CMP_EQ_OQ);

    unsigned even_idx = 0;
    unsigned odd_idx = 1;
    for (unsigned i = 0; i < 4; ++i) {
        if (mask[2 * i]) {
            even_idx = minindices[2 * i];
        }
        if (mask[2 * i + 1]) {
            odd_idx = minindices[2 * i + 1];
        }
    }

    const __m128 x128 =
        _mm_xor_ps(_mm256_extractf128_ps(parity, 1), _mm256_castps256_ps128(parity));
    /* ( -, -, x1+x3+x5+x7, x0+x2+x4+x6 ) */
    const __m128 x64 = _mm_xor_ps(x128, _mm_movehl_ps(x128, x128));

    const unsigned int even_parity = x64[0] > 0 ? 0x00000000 : 0x80000000;
    reinterpret_cast<unsigned int*>(mOutput)[even_idx] ^= even_parity;

    const unsigned int odd_parity = x64[1] > 0 ? 0x00000000 : 0x80000000;
    reinterpret_cast<unsigned int*>(mOutput)[odd_idx] ^= odd_parity;
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
        // std::cout << "SPC: " << blockLength << std::endl;
        return new SpcDecoder(parent);
    }

    // Following are "interleaved one bit unlike the others" codes:
    if (frozenBitCount == blockLength - 2) {
        // std::cout << "DoubleREP: " << blockLength << std::endl;
        // for(auto fb : frozenBits){
        //     std::cout << fb << "\t";
        // }
        // std::cout << std::endl;
        return new DoubleRepetitionDecoder(parent);
    }
    /*
        if (frozenBitCount == 2) {
            std::cout << "DoubleSPC: " << blockLength << std::endl;
            for(auto fb : frozenBits){
                std::cout << fb << "\t";
            }
            std::cout << std::endl;
            return new DoubleSpcDecoder(parent);
        }
    */
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
