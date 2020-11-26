/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/decoding/fastssc_fip_char.h>
#include <polarcode/encoding/butterfly_fip_packed.h>
#include <polarcode/polarcode.h>

#include <iostream>
#include <string>

#include <cmath>
#include <cstring> //for memset

namespace PolarCode {
namespace Decoding {

namespace FastSscFip {


Node::Node()
    : mLlr(nullptr), mBit(nullptr), xmDataPool(nullptr), mBlockLength(0), mVecCount(0)
{
}

Node::Node(Node* parent)
    : mLlr(nullptr),
      mBit(nullptr),
      xmDataPool(parent->pool()),
      mBlockLength(parent->blockLength()),
      mVecCount(nBit2cvecCount(mBlockLength))
{
}

Node::Node(size_t blockLength, datapool_t* pool)
    : mLlr(pool->allocate(nBit2cvecCount(blockLength))),
      mBit(pool->allocate(nBit2cvecCount(blockLength))),
      xmDataPool(pool),
      mBlockLength(blockLength),
      mVecCount(nBit2cvecCount(blockLength))
{
}

Node::~Node()
{
    if (mLlr != nullptr)
        xmDataPool->release(mLlr);
    if (mBit != nullptr)
        xmDataPool->release(mBit);
}

void Node::decode(fipv*, fipv*) {}

size_t Node::blockLength() { return mBlockLength; }

Node::datapool_t* Node::pool() { return xmDataPool; }

fipv* Node::input() { return mLlr->data; }

fipv* Node::output() { return mBit->data; }

ShortNode::ShortNode(Node* parent)
    : Node(parent),
      mLeftBits(xmDataPool->allocate(mVecCount)),
      mRightBits(xmDataPool->allocate(mVecCount))
{
}

ShortNode::~ShortNode()
{
    xmDataPool->release(mLeftBits);
    xmDataPool->release(mRightBits);
}


// Constructors of nodes

RateRNode::RateRNode(const std::vector<unsigned>& frozenBits, Node* parent) : Node(parent)
{
    mBlockLength /= 2;
    mVecCount = nBit2cvecCount(mBlockLength);

    std::vector<unsigned> leftFrozenBits, rightFrozenBits;
    splitFrozenBits(frozenBits, mBlockLength, leftFrozenBits, rightFrozenBits);

    mLeft = createDecoder(leftFrozenBits, this);
    mRight = createDecoder(rightFrozenBits, this);

    ChildLlr = xmDataPool->allocate(mVecCount);
}

ShortRateRNode::ShortRateRNode(const std::vector<unsigned>& frozenBits, Node* parent)
    : RateRNode(frozenBits, parent),
      LeftBits(xmDataPool->allocate(mVecCount)),
      RightBits(xmDataPool->allocate(mVecCount))
{
}

ROneNode::ROneNode(const std::vector<unsigned>& frozenBits, Node* parent)
    : RateRNode(frozenBits, parent)
{
}

ShortROneNode::ShortROneNode(const std::vector<unsigned>& frozenBits, Node* parent)
    : ShortRateRNode(frozenBits, parent)
{
}

ZeroRNode::ZeroRNode(const std::vector<unsigned>& frozenBits, Node* parent)
    : RateRNode(frozenBits, parent)
{
}

ShortZeroRNode::ShortZeroRNode(const std::vector<unsigned>& frozenBits, Node* parent)
    : ShortRateRNode(frozenBits, parent)
{
}

RateZeroDecoder::RateZeroDecoder(Node* parent) : Node(parent) {}

RateOneDecoder::RateOneDecoder(Node* parent) : Node(parent) {}

RepetitionDecoder::RepetitionDecoder(Node* parent) : Node(parent) {}

DoubleRepetitionDecoder::DoubleRepetitionDecoder(Node* parent) : Node(parent) {}

ShortRepetitionDecoder::ShortRepetitionDecoder(Node* parent) : ShortNode(parent) {}

SpcDecoder::SpcDecoder(Node* parent) : Node(parent) {}

ShortSpcDecoder::ShortSpcDecoder(Node* parent) : ShortNode(parent) {}

ZeroSpcDecoder::ZeroSpcDecoder(Node* parent)
    : Node(parent),
      mSubBlockLength(mBlockLength / 2),
      mSubVecCount(nBit2cvecCount(mSubBlockLength))
{
}

ShortZeroSpcDecoder::ShortZeroSpcDecoder(Node* parent)
    : ShortNode(parent), mSubBlockLength(mBlockLength / 2)
{
}

ShortZeroOneDecoder::ShortZeroOneDecoder(Node* parent)
    : ShortNode(parent), mSubBlockLength(mBlockLength / 2)
{
}

// Destructors of nodes

RateRNode::~RateRNode()
{
    if (mLeft)
        delete mLeft;
    if (mRight)
        delete mRight;
    xmDataPool->release(ChildLlr);
}

ShortRateRNode::~ShortRateRNode()
{
    xmDataPool->release(LeftBits);
    xmDataPool->release(RightBits);
}

ROneNode::~ROneNode() {}

ShortROneNode::~ShortROneNode() {}

ZeroRNode::~ZeroRNode() {}

ShortZeroRNode::~ShortZeroRNode() {}

RateZeroDecoder::~RateZeroDecoder() {}

RateOneDecoder::~RateOneDecoder() {}

RepetitionDecoder::~RepetitionDecoder() {}

DoubleRepetitionDecoder::~DoubleRepetitionDecoder() {}

ShortRepetitionDecoder::~ShortRepetitionDecoder() {}

SpcDecoder::~SpcDecoder() {}

ShortSpcDecoder::~ShortSpcDecoder() {}

ZeroSpcDecoder::~ZeroSpcDecoder() {}

ShortZeroSpcDecoder::~ShortZeroSpcDecoder() {}

ShortZeroOneDecoder::~ShortZeroOneDecoder() {}


// Decoders

void RateZeroDecoder::decode(fipv*, fipv* BitsOut)
{
    const fipv inf = fi_set1_epi8(127);
    for (unsigned i = 0; i < mVecCount; ++i) {
        fi_store(BitsOut + i, inf);
    }
}

void RateOneDecoder::decode(fipv* LlrIn, fipv* BitsOut)
{
    for (unsigned i = 0; i < mVecCount; ++i) {
        fi_store(BitsOut + i, fi_load(LlrIn + i));
    }
}


/* WARNING: Saturation can lead to wrong results!

        127 + 127 + 127 + 127 + -128 results in -1 and thus wrong sign
        after step-by-step saturated addition.

        Conversion to epi16 will reduce throughput but circumvents that problem.
*/
void RepetitionDecoder::decode(fipv* LlrIn, fipv* BitsOut)
{
    fipv LlrSum = fi_setzero();

    // Accumulate vectors
    for (unsigned i = 0; i < mVecCount; ++i) {
        LlrSum = fi_adds_epi8(LlrSum, fi_load(LlrIn + i));
    }

    // Get final sum and save decoding result
    char Bits = reduce_adds_epi8(LlrSum);
    LlrSum = fi_set1_epi8(Bits);
    for (unsigned i = 0; i < mVecCount; ++i) {
        fi_store(BitsOut + i, LlrSum);
    }
}

/* WARNING: Saturation can lead to wrong results!

        127 + 127 + 127 + 127 + -128 results in -1 and thus wrong sign
        after step-by-step saturated addition.

        Conversion to epi16 will reduce throughput but circumvents that problem.
*/
void DoubleRepetitionDecoder::decode(fipv* LlrIn, fipv* BitsOut)
{
    fipv LlrSum = fi_setzero();

    // Accumulate vectors
    for (unsigned i = 0; i < mVecCount; ++i) {
        LlrSum = fi_adds_epi8(LlrSum, fi_load(LlrIn + i));
    }

    fipv result = half_reduce_adds_epi8(LlrSum);

    for (unsigned i = 0; i < mVecCount; ++i) {
        fi_store(BitsOut + i, result);
    }
}

void ShortRepetitionDecoder::decode(fipv* LlrIn, fipv* BitsOut)
{
    RepetitionPrepare(LlrIn, mBlockLength);

    // Get sum and save decoding result
    char Bits = reduce_adds_epi8(fi_load(LlrIn));
    fi_store(BitsOut, fi_set1_epi8(Bits));
}

void SpcDecoder::decode(fipv* LlrIn, fipv* BitsOut)
{
    fipv parVec = fi_setzero();
    unsigned minIdx = 0;
    char testAbs, minAbs = 127;

    for (unsigned i = 0; i < mVecCount; i++) {
        fipv vecIn = fi_load(LlrIn + i);
        fi_store(BitsOut + i, vecIn);

        parVec = fi_xor(parVec, vecIn);

        // Only search for minimum if there is a chance for smaller absolute value
        if (minAbs > 0) {
            fipv abs = fi_abs_epi8(vecIn);
            unsigned vecMin = minpos_epu8(abs, &testAbs);
            if (testAbs < minAbs) {
                minIdx = vecMin + i * BYTESPERVECTOR;
                minAbs = testAbs;
            }
        }
    }

    // Flip least reliable bit, if neccessary
    unsigned char parity = reduce_xor(parVec) & 0x80;
    if (parity) {
        char* BitPtr = reinterpret_cast<char*>(BitsOut);
        BitPtr[minIdx] = -BitPtr[minIdx];
    }
}

void ShortSpcDecoder::decode(fipv* LlrIn, fipv* BitsOut)
{
    SpcPrepare(LlrIn, mBlockLength);

    fipv vecIn = fi_load(LlrIn);
    fi_store(BitsOut, vecIn);

    // Flip least reliable bit, if neccessary
    if (reduce_xor(vecIn) & 0x80) {
        fipv abs = fi_abs_epi8(vecIn);
        unsigned vecMin = minpos_epu8(abs);
        unsigned char* BitPtr = reinterpret_cast<unsigned char*>(BitsOut);
        BitPtr[vecMin] = -BitPtr[vecMin];
    }
}

void ZeroSpcDecoder::decode(fipv* LlrIn, fipv* BitsOut)
{
    unsigned char* BitPtr = reinterpret_cast<unsigned char*>(BitsOut);
    fipv parVec = fi_setzero();
    unsigned minIdx = 0;
    char testAbs, minAbs = 127;

    // Check parity equation
    for (unsigned i = 0; i < mSubVecCount; i++) {
        // G-function with only frozen bits
        fipv left = fi_load(LlrIn + i);
        fipv right = fi_load(LlrIn + mSubVecCount + i);
        fipv llr = fi_adds_epi8(left, right);

        // Store output
        fi_store(BitsOut + i, llr);
        fi_store(BitsOut + mSubVecCount + i, llr);

        // Update parity counter
        parVec = fi_xor(parVec, llr);

        // Only search for minimum if there is a chance for smaller absolute value
        if (minAbs > 0) {
            fipv abs = fi_abs_epi8(llr);
            unsigned vecMin = minpos_epu8(abs, &testAbs);
            if (testAbs < minAbs) {
                minIdx = vecMin + i * BYTESPERVECTOR;
                minAbs = testAbs;
            }
        }
    }

    // Flip least reliable bit, if neccessary
    unsigned char parity = reduce_xor(parVec) & 0x80;
    if (parity) {
        BitPtr[minIdx] = -BitPtr[minIdx];
        BitPtr[minIdx + mSubBlockLength] = -BitPtr[minIdx + mSubBlockLength];
    }
}

void ShortZeroSpcDecoder::decode(fipv* LlrIn, fipv* BitsOut)
{
    unsigned char* BitPtr = reinterpret_cast<unsigned char*>(BitsOut);

    // G-function with only frozen bits
    fipv left = fi_load(LlrIn);
    fipv right = subVectorShiftBytes_epu8(left, mSubBlockLength);
    union {
        fipv llr;
        char llr_c[BYTESPERVECTOR];
    };
    llr = fi_adds_epi8(left, right);

    // Set unused bits to SPC-neutral value of 127
    memset(llr_c + mSubBlockLength, 127, BYTESPERVECTOR - mSubBlockLength);

    // Flip least reliable bit, if neccessary
    unsigned char parity = reduce_xor(llr) & 0x80;
    if (parity) {
        fipv abs = fi_abs_epi8(llr);
        unsigned vecMin = minpos_epu8(abs);
        llr_c[vecMin] = -llr_c[vecMin];
    }

    // Generate output
    memcpy(BitPtr, llr_c, mSubBlockLength);
    memcpy(BitPtr + mSubBlockLength, llr_c, mSubBlockLength);
}

void ShortZeroOneDecoder::decode(fipv* LlrIn, fipv* BitsOut)
{
    fipv subLlrLeft, subLlrRight;

    G_function_0RShort(LlrIn, &subLlrLeft, mSubBlockLength);

    subLlrRight = subVectorBackShiftBytes_epu8(subLlrLeft, mSubBlockLength);
    PrepareForShortOperation(&subLlrLeft, mSubBlockLength);
    fi_store(BitsOut, fi_or(subLlrLeft, subLlrRight));
}

void RateRNode::decode(fipv* LlrIn, fipv* BitsOut)
{
    F_function(LlrIn, ChildLlr->data, mBlockLength);

    mLeft->decode(ChildLlr->data, BitsOut);

    G_function(LlrIn, ChildLlr->data, BitsOut, mBlockLength);

    mRight->decode(ChildLlr->data, BitsOut + mVecCount);

    CombineInPlace(BitsOut, mVecCount);
}

void ShortRateRNode::decode(fipv* LlrIn, fipv* BitsOut)
{
    F_function(LlrIn, ChildLlr->data, mBlockLength);

    mLeft->decode(ChildLlr->data, LeftBits->data);

    G_function(LlrIn, ChildLlr->data, LeftBits->data, mBlockLength);

    mRight->decode(ChildLlr->data, RightBits->data);

    CombineBitsShort(LeftBits->data, RightBits->data, BitsOut, mBlockLength);
}

void ROneNode::decode(fipv* LlrIn, fipv* BitsOut)
{
    F_function(LlrIn, ChildLlr->data, mBlockLength);

    mLeft->decode(ChildLlr->data, BitsOut);

    simplifiedRightRateOneDecode(LlrIn, BitsOut);
}

void ROneNode::simplifiedRightRateOneDecode(fipv* LlrIn, fipv* BitsOut)
{
    for (unsigned i = 0; i < mVecCount; ++i) {
        fipv Llr_l = fi_load(LlrIn + i);
        fipv Llr_r = fi_load(LlrIn + i + mVecCount);
        fipv Bits = fi_load(BitsOut + i);
        fipv Llr_o;

        G_function_calc(Llr_l, Llr_r, Bits, &Llr_o);
        /*nop*/                                     // Rate 1 decoder
        fi_store(BitsOut + i, fi_xor(Bits, Llr_o)); // Combine left bit
        fi_store(BitsOut + i + mVecCount, Llr_o);   // Copy right bit
    }
}

void ShortROneNode::decode(fipv* LlrIn, fipv* BitsOut)
{
    F_function(LlrIn, ChildLlr->data, mBlockLength);

    mLeft->decode(ChildLlr->data, BitsOut);

    simplifiedRightRateOneDecodeShort(LlrIn, BitsOut);
}

void ShortROneNode::simplifiedRightRateOneDecodeShort(fipv* LlrIn, fipv* BitsOut)
{
    fipv Bits = fi_load(BitsOut);      // Load left bits
    fipv Llr_r_subcode = fi_setzero(); // Destination for right subcode

    G_function(LlrIn, &Llr_r_subcode, BitsOut, mBlockLength); // Get right child LLRs
    /*nop*/                                                   // Rate 1 decoder
    fipv Bits_r = subVectorBackShiftBytes_epu8(Llr_r_subcode, mBlockLength);
    fipv Bits_o = fi_xor(Bits, Llr_r_subcode); // Combine left bits
    memset(reinterpret_cast<char*>(&Bits_o) + mBlockLength,
           0,
           mBlockLength);         // Clear right bits
    Bits = fi_or(Bits_o, Bits_r); // Merge bits into single vector
    fi_store(BitsOut, Bits);      // Save
}

void ZeroRNode::decode(fipv* LlrIn, fipv* BitsOut)
{
    G_function_0R(LlrIn, ChildLlr->data, mBlockLength);

    mRight->decode(ChildLlr->data, BitsOut + mVecCount);

    Combine_0R(BitsOut, mBlockLength);
}

void ShortZeroRNode::decode(fipv* LlrIn, fipv* BitsOut)
{
    G_function_0RShort(LlrIn, ChildLlr->data, mBlockLength);

    mRight->decode(ChildLlr->data, RightBits->data);

    Combine_0RShort(BitsOut, RightBits->data, mBlockLength);
}

// End of mass defining

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
        if (blockLength <= BYTESPERVECTOR) {
            return new ShortRepetitionDecoder(parent);
        } else {
            return new RepetitionDecoder(parent);
        }
    }
    if (frozenBitCount == 1) {
        if (blockLength <= BYTESPERVECTOR) {
            return new ShortSpcDecoder(parent);
        } else {
            return new SpcDecoder(parent);
        }
    }

    if (frozenBitCount == blockLength - 2 and blockLength >= BYTESPERVECTOR) {
        return new DoubleRepetitionDecoder(parent);
    }

    // Precalculate subcodes to find special child node combinations
    std::vector<unsigned> leftFrozenBits, rightFrozenBits;
    splitFrozenBits(frozenBits, blockLength / 2, leftFrozenBits, rightFrozenBits);

    if (blockLength <= BYTESPERVECTOR) {
        if (leftFrozenBits.size() == blockLength / 2 && rightFrozenBits.size() == 0) {
            return new ShortZeroOneDecoder(parent);
        }

        if (leftFrozenBits.size() == blockLength / 2 && rightFrozenBits.size() == 1) {
            return new ShortZeroSpcDecoder(parent);
        }

        // Fallback: No special decoder available
        if (rightFrozenBits.size() == 0) {
            return new ShortROneNode(frozenBits, parent);
        }

        if (leftFrozenBits.size() == blockLength / 2) {
            return new ShortZeroRNode(frozenBits, parent);
        }

        return new ShortRateRNode(frozenBits, parent);
    } else {
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

} // namespace FastSscFip

FastSscFipChar::FastSscFipChar(size_t blockLength,
                               const std::vector<unsigned>& frozenBits)
{
    initialize(blockLength, frozenBits);
}

FastSscFipChar::~FastSscFipChar() { clear(); }

void FastSscFipChar::clear()
{
    delete mEncoder;
    delete mRootNode;
    delete mNodeBase;
    delete mDataPool;
}

void FastSscFipChar::initialize(size_t blockLength,
                                const std::vector<unsigned>& frozenBits)
{
    if (blockLength == mBlockLength && frozenBits == mFrozenBits) {
        return;
    }
    if (mBlockLength != 0) {
        clear();
    }
    mBlockLength = blockLength;
    mFrozenBits.assign(frozenBits.begin(), frozenBits.end());

    mEncoder = new Encoding::ButterflyFipPacked(mBlockLength, mFrozenBits);
    mEncoder->setSystematic(false);

    mDataPool = new DataPool<fipv, BYTESPERVECTOR>();
    mNodeBase = new FastSscFip::Node(blockLength, mDataPool);
    mRootNode = FastSscFip::createDecoder(frozenBits, mNodeBase);
    mLlrContainer =
        new CharContainer(reinterpret_cast<char*>(mNodeBase->input()), mBlockLength);
    mBitContainer =
        new CharContainer(reinterpret_cast<char*>(mNodeBase->output()), mBlockLength);
    mLlrContainer->setFrozenBits(mFrozenBits);
    mBitContainer->setFrozenBits(mFrozenBits);
    mOutputContainer = new unsigned char[(mBlockLength - frozenBits.size() + 7) / 8];
}

bool FastSscFipChar::decode()
{
    mRootNode->decode(mNodeBase->input(), mNodeBase->output());
    if (!mSystematic) {
        mEncoder->setCharCodeword(dynamic_cast<CharContainer*>(mBitContainer)->data());
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
