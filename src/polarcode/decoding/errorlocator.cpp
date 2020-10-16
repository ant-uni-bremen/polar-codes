/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/decoding/avx_float.h>
#include <polarcode/decoding/errorlocator.h>
#include <polarcode/polarcode.h>


namespace PolarCode {
namespace Decoding {

namespace ErrorLocatorNodes {

Node::Node() {}

Node::Node(unsigned blockLength, datapool_t* pool, ErrorLocator* decoder)
    : xmDecoder(decoder),
      mBlockLength(blockLength),
      xmDataPool(pool),
      mLlr(pool->allocate(blockLength)),
      mBit(pool->allocate(blockLength)),
      mInput(mLlr->data),
      mOutput(mBit->data)
{
}

Node::Node(Node* other)
    : xmDecoder(other->xmDecoder),
      mBlockLength(other->mBlockLength),
      xmDataPool(other->xmDataPool),
      mLlr(xmDataPool->allocate(mBlockLength)),
      mBit(xmDataPool->allocate(mBlockLength)),
      mInput(other->mInput),
      mOutput(other->mOutput)
{
}

Node::~Node()
{
    xmDataPool->release(mLlr);
    xmDataPool->release(mBit);
}

unsigned Node::blockLength() { return mBlockLength; }

ErrorLocator* Node::decoder() { return xmDecoder; }

void Node::setInput(float* input) { mInput = input; }

void Node::setOutput(float* output) { mOutput = output; }

float* Node::input() { return mInput; }

float* Node::output() { return mOutput; }

void Node::decode()
{
    // Must be implemented for the base node.
    // Won't be actually called.
}

/*************
 * RateRNode
 * ***********/

RateRNode::RateRNode(const std::vector<unsigned>& frozenBits, Node* parent) : Node(parent)
{
    mBlockLength /= 2;

    mLeftLlr = xmDataPool->allocate(mBlockLength);
    mRightLlr = xmDataPool->allocate(mBlockLength);

    std::vector<unsigned> leftFrozenBits, rightFrozenBits;
    splitFrozenBits(frozenBits, mBlockLength, leftFrozenBits, rightFrozenBits);

    mLeft = createDecoder(leftFrozenBits, this);
    mLeft->setInput(mLeftLlr->data);
    mLeft->setOutput(mOutput);

    mRight = createDecoder(rightFrozenBits, this);
    mRight->setInput(mRightLlr->data);
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
    FastSscAvx::F_function(mInput, mLeftLlr->data, mBlockLength);
    mLeft->decode();
    FastSscAvx::G_function(mInput, mRightLlr->data, mOutput, mBlockLength);
    mRight->decode();
    FastSscAvx::Combine(mOutput, mBlockLength);
}

/*************
 * ShortRateRNode
 * ***********/

ShortRateRNode::ShortRateRNode(const std::vector<unsigned>& frozenBits, Node* parent)
    : RateRNode(frozenBits, parent)
{
    mLeftBits = xmDataPool->allocate(mBlockLength);
    mRightBits = xmDataPool->allocate(mBlockLength);
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
    FastSscAvx::F_function(mInput, mLeftLlr->data, mBlockLength);
    mLeft->decode();
    FastSscAvx::G_function(mInput, mRightLlr->data, mLeftBits->data, mBlockLength);
    mRight->decode();
    FastSscAvx::CombineBitsShort(
        mLeftBits->data, mRightBits->data, mOutput, mBlockLength);
}

/*************
 * Bit
 * ***********/

Bit::Bit(Node* parent) : Node(parent) {}

Bit::~Bit() {}

void Bit::decode() { mValue = *mOutput = mReplace ? mDesiredValue : *mInput; }


Node* createDecoder(const std::vector<unsigned>& frozenBits, Node* parent)
{
    unsigned blockLength = parent->blockLength();
    unsigned frozenBitCount = frozenBits.size();

    if (blockLength == 1) {
        Bit* node = new Bit(parent);
        parent->decoder()->pushBit(node, frozenBitCount == 1);
        return node;
    }

    if (blockLength <= 8) {
        return new ShortRateRNode(frozenBits, parent);
    } else {
        return new RateRNode(frozenBits, parent);
    }
}

} // namespace ErrorLocatorNodes

ErrorLocator::ErrorLocator(unsigned blockLength, const std::vector<unsigned>& frozenBits)
{
    mIsReferenceDecoder = false;
    initialize(blockLength, frozenBits);
}

ErrorLocator::~ErrorLocator() { clear(); }

void ErrorLocator::clear()
{
    delete mRootNode;
    delete mNodeBase;
    mSystematicNodes.clear();
    mInfoBits.clear();
}

void ErrorLocator::initialize(unsigned blockLength,
                              const std::vector<unsigned>& frozenBits)
{
    if (mBlockLength == blockLength && frozenBits == mFrozenBits) {
        return;
    }
    if (mBlockLength != 0) {
        clear();
    }
    mBlockLength = blockLength;
    mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
    mDataPool = new ErrorLocatorNodes::datapool_t();
    mNodeBase = new ErrorLocatorNodes::Node(blockLength, mDataPool, this);
    mRootNode = ErrorLocatorNodes::createDecoder(mFrozenBits, mNodeBase);
    mLlrContainer = new FloatContainer(mNodeBase->input(), mBlockLength);
    mBitContainer = new FloatContainer(mNodeBase->output(), mBlockLength);
    mLlrContainer->setFrozenBits(mFrozenBits);
    mBitContainer->setFrozenBits(mFrozenBits);
}

void ErrorLocator::setAsReferenceDecoder() { mIsReferenceDecoder = true; }

bool ErrorLocator::decode()
{
    if (mIsReferenceDecoder) {
        mRootNode->decode();
    } else {
        prepare();
        do {
            mRootNode->decode();
        } while (findErrors());
    }
    return true;
}

int ErrorLocator::decodeFindFirstError()
{
    prepare();
    mRootNode->decode();
    return findFirstError();
}

void ErrorLocator::pushBit(ErrorLocatorNodes::Node* node, bool frozen)
{
    mSystematicNodes.push_back(node);
    node->mId = mSystematicNodes.size() - 1;

    if (frozen) {
        node->mDesiredValue = INFINITY;
        node->mReplace = true;
    } else {
        node->mDesiredValue = 0.0f;
        node->mReplace = false;
        mInfoBits.push_back(node);
    }
}


void ErrorLocator::prepare()
{
    mFirstError = -1;
    mCorrectionCount = 0;
    for (auto node : mInfoBits) {
        node->mReplace = false;
    }
}

inline bool signEq(HybridFloat a, HybridFloat b) { return !((a.u ^ b.u) & 0x80000000U); }

bool ErrorLocator::findErrors()
{
    HybridFloat desired, actual;
    for (auto node : mInfoBits) {
        desired.f = node->mDesiredValue;
        actual.f = node->mValue;

        if (!signEq(desired, actual) && !node->mReplace) {
            node->mReplace = true;

            if (mCorrectionCount == 0) {
                mFirstError = node->mId;
            }
            mCorrectionCount++;
            return true;
        }
    }
    return false;
}

int ErrorLocator::findFirstError()
{
    HybridFloat desired, actual;
    for (auto node : mInfoBits) {
        desired.f = node->mDesiredValue;
        actual.f = node->mValue;

        if (!signEq(desired, actual) && !node->mReplace) {
            mFirstError = node->mId;
            return mFirstError;
        }
    }
    return -1;
}

std::vector<float> ErrorLocator::getOutput()
{
    std::vector<float> output;
    output.resize(mBlockLength);

    for (unsigned i = 0; i < mBlockLength; ++i) {
        output[i] = mSystematicNodes[i]->mValue;
    }

    return output;
}

void ErrorLocator::setDesiredOutput(std::vector<float> desired)
{
    for (unsigned i = 0; i < mBlockLength; ++i) {
        mSystematicNodes[i]->mDesiredValue = desired[i];
    }
}

int ErrorLocator::firstError() { return mFirstError; }

int ErrorLocator::correctionCount() { return mCorrectionCount; }


} // namespace Decoding
} // namespace PolarCode
