/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/encoding/butterfly_fip.h>
#include <polarcode/encoding/recursive_fip_packed.h>
#include <polarcode/polarcode.h>
#include <stdlib.h>

namespace PolarCode {
namespace Encoding {

namespace RecursiveFip {

Node::Node() : mBit(nullptr), mBlockLength(0), mVecCount(0) {}

Node::Node(size_t blockLength)
    : mBlockLength(blockLength), mVecCount(nBit2vecCount(blockLength))
{
<<<<<<< HEAD
    mBit = reinterpret_cast<fipv*>(
        aligned_alloc(BYTESPERVECTOR, mVecCount * BYTESPERVECTOR));
=======
    mBit =
        reinterpret_cast<fipv*>(aligned_alloc(BYTESPERVECTOR, mVecCount * BYTESPERVECTOR));
>>>>>>> b62149fa768fbfd54a1abeef435ecf7946b8791d
}

Node::~Node() { clearBlock(); }

void Node::encode(fipv* Bits) { throw "This should not be called."; }

void Node::clearBlock()
{
    if (mBit != nullptr) {
        free(mBit);
        mBit = nullptr;
        mBlockLength = 0;
        mVecCount = 0;
    }
}

void Node::unsetBlockPointer() { mBit = nullptr; }

size_t Node::blockLength() { return mBlockLength; }

fipv* Node::block() { return mBit; }

// Constructors

RateOneNode::RateOneNode(Node* parent) : mParent(parent) {}

RateZeroNode::RateZeroNode(Node* parent) : mParent(parent)
{
    mBlockLength = parent->blockLength();
    mVecCount = nBit2vecCount(mBlockLength);
}

RepetitionNode::RepetitionNode(Node* parent) : mParent(parent)
{
    mBlockLength = parent->blockLength();
    mVecCount = nBit2vecCount(mBlockLength);
}

SpcNode::SpcNode(Node* parent) : mParent(parent)
{
    mBlockLength = parent->blockLength();
    mVecCount = nBit2vecCount(mBlockLength);
}

ShortButterflyNode::ShortButterflyNode(std::vector<unsigned>& frozenBits, Node* parent)
    : mParent(parent)
{
    mBlockLength = parent->blockLength();
    mVecCount = nBit2vecCount(mBlockLength);
    mButterflyEncoder = new ButterflyFipPacked(mBlockLength, frozenBits);
}

RateRNode::RateRNode(std::vector<unsigned>& frozenBits, Node* parent)
    : Node(), mParent(parent)
{
    mBlockLength = parent->blockLength() / 2;
    mVecCount = nBit2vecCount(mBlockLength);
    mStage = __builtin_ctz(mBlockLength);

    std::vector<unsigned> leftFrozenBits, rightFrozenBits;
    splitFrozenBits(frozenBits, mBlockLength, leftFrozenBits, rightFrozenBits);

    mLeft = createEncoder(leftFrozenBits, this);
    mRight = createEncoder(rightFrozenBits, this);
}

// Destructors

RateOneNode::~RateOneNode() {}

RateZeroNode::~RateZeroNode() {}

RepetitionNode::~RepetitionNode() {}

SpcNode::~SpcNode() {}

ShortButterflyNode::~ShortButterflyNode() { delete mButterflyEncoder; }

RateRNode::~RateRNode()
{
    delete mLeft;
    delete mRight;
}

// Encoders

void RateOneNode::encode(fipv* Bits) {}

void RateZeroNode::encode(fipv* Bits)
{
    const fipv zero = fi_setzero();
    for (unsigned i = 0; i < mVecCount; ++i) {
        fi_store(Bits + i, zero);
    }
}

void RepetitionNode::encode(fipv* Bits)
{
    const char bit =
        reinterpret_cast<char*>(Bits + mVecCount - 1)[BYTESPERVECTOR - 1] & 1;
    const fipv vector = fi_set1_epi8(0 - bit);
    for (unsigned i = 0; i < mVecCount; ++i) {
        fi_store(Bits + i, vector);
    }
}

void SpcNode::encode(fipv* Bits)
{
    // clear the parity bit
    unsigned char* firstByte = reinterpret_cast<unsigned char*>(Bits);
    *firstByte &= 0x7F;

    fipv parVec = fi_setzero();
    for (unsigned i = 0; i < mVecCount; ++i) {
        parVec = fi_xor(parVec, fi_load(Bits + i));
    }
    unsigned char parity = reduce_xor(parVec);
    // Above reduce operation stops at eight bit width
    // We need to reduce further to single bit
    parity ^= parity << 4;
    parity ^= parity << 2;
    parity ^= parity << 1;
    parity &= 0x80;       // Clear unused bits
    *firstByte |= parity; // Insert parity information
}

void ShortButterflyNode::encode(fipv* Bits)
{
    mButterflyEncoder->setCodeword(Bits);
    mButterflyEncoder->clearFrozenBits();
    mButterflyEncoder->encode();
    mButterflyEncoder->getEncodedData(Bits);
}

void RateRNode::encode(fipv* Bits)
{
    fipv* rightBits = Bits + mVecCount;

    mRight->encode(rightBits);

    ButterflyFipPackedTransform(Bits, mBlockLength * 2, mStage);
    mLeft->encode(Bits);
    ButterflyFipPackedTransform(Bits, mBlockLength * 2, mStage);
}

// End of nodes

Node* createEncoder(std::vector<unsigned>& frozenBits, Node* parent)
{
    size_t blockLength = parent->blockLength();
    size_t frozenBitCount = frozenBits.size();

    // Straightforward codes
    if (frozenBitCount == 0) {
        return new RateOneNode(parent);
    }
    if (frozenBitCount == blockLength) {
        return new RateZeroNode(parent);
    }

    // "One bit unlike the others"
    if (frozenBitCount == (blockLength - 1)) {
        return new RepetitionNode(parent);
    }
    if (frozenBitCount == 1) {
        return new SpcNode(parent);
    }

    // General codes
    if (blockLength == BITSPERVECTOR) {
        // No specializations for half-length codes
        return new ShortButterflyNode(frozenBits, parent);
    } else {
        // Divide code into half-length subcodes
        return new RateRNode(frozenBits, parent);
    }
}

size_t nBit2vecCount(size_t blockLength)
{
    return (blockLength + (BITSPERVECTOR - 1)) / BITSPERVECTOR;
}

} // namespace RecursiveFip

RecursiveFipPacked::RecursiveFipPacked(size_t blockLength,
                                       const std::vector<unsigned>& frozenBits)
{
    if (blockLength < BITSPERVECTOR) {
        throw "Recursive encoder does not exist for block length smaller than vector "
              "size!";
        return;
    }
    mBlockLength = 0;
    initialize(blockLength, frozenBits);
}

RecursiveFipPacked::~RecursiveFipPacked() { clear(); }

void RecursiveFipPacked::clear()
{
    delete mBitContainer;
    mBitContainer = nullptr;
    delete mRootNode;
    mRootNode = nullptr;
    delete mNodeBase;
    mNodeBase = nullptr;
}

void RecursiveFipPacked::initialize(size_t blockLength,
                                    const std::vector<unsigned>& frozenBits)
{
    if (blockLength == mBlockLength && frozenBits == mFrozenBits) {
        return;
    } else {
        if (mBlockLength != 0) {
            clear();
        }
        mBlockLength = blockLength;
        mFrozenBits = frozenBits;
        mNodeBase = new RecursiveFip::Node(blockLength);
        mRootNode = RecursiveFip::createEncoder(mFrozenBits, mNodeBase);
        mBitContainer = new PackedContainer(
            reinterpret_cast<char*>(mNodeBase->block()), mBlockLength, mFrozenBits);
    }
}

void RecursiveFipPacked::encode()
{
    mErrorDetector->generate(xmInputData, (mBlockLength - mFrozenBits.size()) / 8);
    mBitContainer->insertPackedInformationBits(xmInputData);

    mRootNode->encode(mNodeBase->block());
}
} // namespace Encoding
} // namespace PolarCode
