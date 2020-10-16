/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/arrayfuncs.h>
#include <polarcode/decoding/scl_fip_char.h>
#include <polarcode/encoding/butterfly_fip_packed.h>
#include <polarcode/polarcode.h>

namespace PolarCode {
namespace Decoding {

namespace SclFip {

PathList::PathList() {}

PathList::PathList(size_t listSize, size_t stageCount, datapool_t* dataPool)
    : mPathLimit(listSize),
      mPathCount(0),
      mNextPathCount(0),
      mStageCount(stageCount),
      xmDataPool(dataPool)
{
    mLlrTree.resize(listSize);
    mBitTree.resize(listSize);
    mLeftBitTree.resize(listSize);
    mMetric.assign(listSize, 0);
    mNextLlrTree.resize(listSize);
    mNextBitTree.resize(listSize);
    mNextLeftBitTree.resize(listSize);
    mNextMetric.assign(listSize, 0);

    for (unsigned i = 0; i < mPathLimit; ++i) {
        mLlrTree[i].resize(stageCount);
        mBitTree[i].resize(stageCount);
        mLeftBitTree[i].resize(stageCount);
        mNextLlrTree[i].resize(stageCount);
        mNextBitTree[i].resize(stageCount);
        mNextLeftBitTree[i].resize(stageCount);
    }
}

PathList::~PathList() { clear(); }

void PathList::clear()
{
    clearStage(mStageCount - 1);
    mPathCount = 0;
}

void PathList::duplicatePath(unsigned destination, unsigned source, unsigned stage)
{
    for (unsigned i = stage; i < mStageCount; ++i) {
        mNextLlrTree[destination][i] = xmDataPool->lazyDuplicate(mLlrTree[source][i]);
        mNextBitTree[destination][i] = xmDataPool->lazyDuplicate(mBitTree[source][i]);
        mNextLeftBitTree[destination][i] =
            xmDataPool->lazyDuplicate(mLeftBitTree[source][i]);
    }
}

void PathList::getWriteAccessToLlr(unsigned path, unsigned stage)
{
    xmDataPool->prepareForWrite(mLlrTree[path][stage]);
}

void PathList::getWriteAccessToBit(unsigned path, unsigned stage)
{
    xmDataPool->prepareForWrite(mBitTree[path][stage]);
}

void PathList::getWriteAccessToNextBit(unsigned path, unsigned stage)
{
    xmDataPool->prepareForWrite(mNextBitTree[path][stage]);
}

void PathList::clearOldPaths(unsigned stage)
{
    for (unsigned path = 0; path < mPathCount; ++path) {
        for (unsigned i = stage; i < mStageCount; ++i) {
            xmDataPool->release(mLlrTree[path][i]);
            xmDataPool->release(mBitTree[path][i]);
            xmDataPool->release(mLeftBitTree[path][i]);
        }
    }
}

void PathList::switchToNext()
{
    std::swap(mLlrTree, mNextLlrTree);
    std::swap(mBitTree, mNextBitTree);
    std::swap(mLeftBitTree, mNextLeftBitTree);
    std::swap(mMetric, mNextMetric);
    mPathCount = mNextPathCount;
}

void PathList::setFirstPath(void* pLlr)
{
    mPathCount = 1;
    unsigned stage = mStageCount - 1;
    allocateStage(stage);

    memcpy(Llr(0, stage), pLlr, 1 << stage);
}

void PathList::allocateStage(unsigned stage)
{
    unsigned vecCount = nBit2cvecCount(1 << stage);
    for (unsigned path = 0; path < mPathCount; ++path) {
        mLlrTree[path][stage] = xmDataPool->allocate(vecCount);
        mBitTree[path][stage] = xmDataPool->allocate(vecCount);
        mLeftBitTree[path][stage] = xmDataPool->allocate(vecCount);
    }
}

void PathList::clearStage(unsigned stage)
{
    for (unsigned path = 0; path < mPathCount; ++path) {
        xmDataPool->release(mLlrTree[path][stage]);
        xmDataPool->release(mBitTree[path][stage]);
        xmDataPool->release(mLeftBitTree[path][stage]);
    }
}

fipv* PathList::Llr(unsigned path, unsigned stage) { return mLlrTree[path][stage]->data; }

fipv* PathList::Bit(unsigned path, unsigned stage) { return mBitTree[path][stage]->data; }

fipv* PathList::LeftBit(unsigned path, unsigned stage)
{
    return mLeftBitTree[path][stage]->data;
}

void PathList::prepareRightDecoding(unsigned stage)
{
    for (unsigned path = 0; path < mPathCount; ++path) {
        std::swap(mBitTree[path][stage], mLeftBitTree[path][stage]);
        xmDataPool->prepareForWrite(mLlrTree[path][stage]);
    }
}

fipv* PathList::NextLlr(unsigned path, unsigned stage)
{
    return mNextLlrTree[path][stage]->data;
}

fipv* PathList::NextBit(unsigned path, unsigned stage)
{
    return mNextBitTree[path][stage]->data;
}

long& PathList::Metric(unsigned path) { return mMetric[path]; }

long& PathList::NextMetric(unsigned path) { return mNextMetric[path]; }

unsigned PathList::PathCount() { return mPathCount; }

unsigned PathList::PathLimit() { return mPathLimit; }

void PathList::setNextPathCount(unsigned pc) { mNextPathCount = pc; }

Node::Node() {}

Node::Node(Node* other)
    : xmDataPool(other->xmDataPool),
      mBlockLength(other->mBlockLength),
      mVecCount(other->mVecCount),
      mStage(other->mStage),
      mListSize(other->mListSize),
      xmPathList(other->xmPathList)
{
}

Node::Node(size_t blockLength, size_t listSize, datapool_t* pool, PathList* pathList)
    : xmDataPool(pool),
      mBlockLength(blockLength),
      mVecCount(nBit2cvecCount(blockLength)),
      mStage(__builtin_ctz(mBlockLength)),
      mListSize(listSize),
      xmPathList(pathList)
{
}

Node::~Node() {}

void Node::decode() {}

datapool_t* Node::pool() { return xmDataPool; }

unsigned Node::blockLength() { return mBlockLength; }

unsigned Node::listSize() { return mListSize; }

SclFip::PathList* Node::pathList() { return xmPathList; }


// Constructors

RateRNode::RateRNode(const std::vector<unsigned>& frozenBits, Node* parent) : Node(parent)
{
    mBlockLength /= 2;
    mStage -= 1;
    mVecCount = nBit2cvecCount(mBlockLength);

    std::vector<unsigned> leftFrozenBits, rightFrozenBits;
    splitFrozenBits(frozenBits, mBlockLength, leftFrozenBits, rightFrozenBits);

    mLeft = createDecoder(leftFrozenBits, this);
    mRight = createDecoder(rightFrozenBits, this);
}

ShortRateRNode::ShortRateRNode(const std::vector<unsigned>& frozenBits, Node* parent)
    : RateRNode(frozenBits, parent)
{
}

RateZeroDecoder::RateZeroDecoder(Node* parent) : Node(parent)
{
    mIndices.resize(mListSize);
    mMetrics.resize(mListSize);
}

RateOneDecoder::RateOneDecoder(Node* parent) : Node(parent)
{
    mIndices.resize(std::max(mBlockLength, mListSize * 4));
    mMetrics.resize(mListSize * 4);
    mBitFlipHints.resize(mListSize * 4);
    mBitFlipCount.resize(mListSize * 4);
}

RepetitionDecoder::RepetitionDecoder(Node* parent) : Node(parent)
{
    mIndices.resize(mListSize * 2);
    mMetrics.resize(mListSize * 2);
    mResults.resize(mListSize * 2);
}

SpcDecoder::SpcDecoder(Node* parent) : Node(parent)
{
    mIndices.resize(std::max(std::max(mBlockLength, mListSize * 8), 32u));
    mMetrics.resize(mListSize * 8);
    mBitFlipHints.resize(mListSize * 8);
    mBitFlipCount.resize(mListSize * 8);
}

// Destructors

RateRNode::~RateRNode()
{
    if (mLeft)
        delete mLeft;
    if (mRight)
        delete mRight;
}

ShortRateRNode::~ShortRateRNode() {}

RateZeroDecoder::~RateZeroDecoder() {}

RateOneDecoder::~RateOneDecoder() {}

RepetitionDecoder::~RepetitionDecoder() {}

SpcDecoder::~SpcDecoder() {}

// Decoders
#ifndef __AVX2__
inline __m128i llrExpandToLong(__m128i vec, const unsigned i)
{
    switch (i) {
    case 0:
        break;
    case 1:
        vec = _mm_shuffle_epi32(vec, 1);
        break;
    case 2:
        vec = _mm_shuffle_epi32(vec, 2);
        break;
    case 3:
        vec = _mm_shuffle_epi32(vec, 3);
        break;
    }

    return _mm_cvtepi8_epi64(vec);
}

#else
inline __m256i llrExpandToLong(fipv vec, const unsigned i)
{
    switch (i % 4) {
    case 0:
        break;
    case 1:
        vec = _mm256_shuffle_epi32(vec, 1);
        break;
    case 2:
        vec = _mm256_shuffle_epi32(vec, 2);
        break;
    case 3:
        vec = _mm256_shuffle_epi32(vec, 3);
        break;
    }

    if (i / 4) {
        return _mm256_cvtepi8_epi64(_mm256_extracti128_si256(vec, 1));
    } else {
        return _mm256_cvtepi8_epi64(_mm256_extracti128_si256(vec, 0));
    }
}
#endif

void RateRNode::decode()
{
    xmPathList->allocateStage(mStage);

    unsigned pathCount = xmPathList->PathCount();
    for (unsigned path = 0; path < pathCount; ++path) {
        FastSscFip::F_function(xmPathList->Llr(path, mStage + 1),
                               xmPathList->Llr(path, mStage),
                               mBlockLength);
    }

    mLeft->decode();

    xmPathList->prepareRightDecoding(mStage);
    pathCount = xmPathList->PathCount();
    for (unsigned path = 0; path < pathCount; ++path) {
        FastSscFip::G_function(xmPathList->Llr(path, mStage + 1),
                               xmPathList->Llr(path, mStage),
                               xmPathList->LeftBit(path, mStage),
                               mBlockLength);
    }

    mRight->decode();

    pathCount = xmPathList->PathCount();
    for (unsigned path = 0; path < pathCount; ++path) {
        xmPathList->getWriteAccessToBit(path, mStage + 1);
        FastSscFip::CombineBits(xmPathList->LeftBit(path, mStage),
                                xmPathList->Bit(path, mStage),
                                xmPathList->Bit(path, mStage + 1),
                                mBlockLength);
    }

    xmPathList->clearStage(mStage);
}

void ShortRateRNode::decode()
{
    xmPathList->allocateStage(mStage);

    unsigned pathCount = xmPathList->PathCount();
    for (unsigned path = 0; path < pathCount; ++path) {
        FastSscFip::F_function(xmPathList->Llr(path, mStage + 1),
                               xmPathList->Llr(path, mStage),
                               mBlockLength);
    }

    mLeft->decode();

    xmPathList->prepareRightDecoding(mStage);
    pathCount = xmPathList->PathCount();
    for (unsigned path = 0; path < pathCount; ++path) {
        FastSscFip::G_function(xmPathList->Llr(path, mStage + 1),
                               xmPathList->Llr(path, mStage),
                               xmPathList->LeftBit(path, mStage),
                               mBlockLength);
    }

    mRight->decode();

    pathCount = xmPathList->PathCount();
    for (unsigned path = 0; path < pathCount; ++path) {
        xmPathList->getWriteAccessToBit(path, mStage + 1);
        FastSscFip::CombineBitsShort(xmPathList->LeftBit(path, mStage),
                                     xmPathList->Bit(path, mStage),
                                     xmPathList->Bit(path, mStage + 1),
                                     mBlockLength);
    }

    xmPathList->clearStage(mStage);
}

void RateZeroDecoder::decode()
{
    const fipv zero = fi_setzero();
    const fipv inf = fi_set1_epi8(127);
    unsigned pathCount = xmPathList->PathCount();
    fipv* bitDestination;
    union {
        fipv* vLlrSource;
        char* cLlrSource;
    };

    for (unsigned path = 0; path < pathCount; ++path) {
        fipv punishment = fi_setzero();
        fipv expanded;
        vLlrSource = xmPathList->Llr(path, mStage);
        bitDestination = xmPathList->Bit(path, mStage);

        for (unsigned i = mBlockLength; i < BYTESPERVECTOR; ++i) {
            cLlrSource[i] = 0;
        }

        for (unsigned vector = 0; vector < mVecCount; ++vector) {
            fi_store(bitDestination + vector, inf);

            fipv LlrIn = fi_load(vLlrSource + vector);
            LlrIn = fi_min_epi8(LlrIn, zero);

            for (unsigned group = 0; group < BYTESPERVECTOR / 4; ++group) {
                expanded = llrExpandToLong(LlrIn, group);
                punishment = fi_add_epi64(punishment, expanded);
            }
        }
        xmPathList->Metric(path) += reduce_add_epi64(punishment);
    }
}

void RateOneDecoder::decode()
{
    const fipv absCorrector = fi_set1_epi8(-127);
    unsigned pathCount = xmPathList->PathCount();
    Block<fipv>* block = xmDataPool->allocate(mVecCount);
    union {
        fipv* vTempBlock;
        char* cTempBlock;
    };
    union {
        fipv* vLlrSource;
        char* cLlrSource;
    };

    vTempBlock = block->data;

    for (unsigned path = 0; path < pathCount; ++path) {
        long metric = xmPathList->Metric(path);
        vLlrSource = xmPathList->Llr(path, mStage);

        for (unsigned i = mBlockLength; i < BYTESPERVECTOR; ++i) {
            cLlrSource[i] = 127;
        }

        for (unsigned i = 0; i < mVecCount; ++i) {
            fipv Llr = fi_load(vLlrSource + i);
            Llr = fi_abs_epi8(fi_max_epi8(Llr, absCorrector));
            fi_store(vTempBlock + i, Llr);
        }
        findWeakLlrs(mIndices, cTempBlock, mBlockLength, 2);
        mMetrics[path * 4] = metric;
        mMetrics[path * 4 + 1] = metric - cTempBlock[0];
        if (mBlockLength == 1) {
            mMetrics[path * 4 + 2] = -0x100000000000LL;
            mMetrics[path * 4 + 3] = -0x100000000000LL;
        } else {
            mMetrics[path * 4 + 2] = metric - cTempBlock[1];
            mMetrics[path * 4 + 3] = metric - cTempBlock[0] - cTempBlock[1];
        }

        mBitFlipHints[path * 4 + 1][0] = mIndices[0];
        mBitFlipHints[path * 4 + 2][0] = mIndices[1];
        mBitFlipHints[path * 4 + 3][0] = mIndices[0];
        mBitFlipHints[path * 4 + 3][1] = mIndices[1];

        mBitFlipCount[path * 4] = 0;
        mBitFlipCount[path * 4 + 1] = 1;
        mBitFlipCount[path * 4 + 2] = 1;
        mBitFlipCount[path * 4 + 3] = 2;
    }
    xmDataPool->release(block);

    unsigned newPathCount = std::min(pathCount * 4, xmPathList->PathLimit());
    xmPathList->setNextPathCount(newPathCount);
    simplePartialSortDescending(mIndices, mMetrics, newPathCount, pathCount * 4);

    for (unsigned path = 0; path < newPathCount; ++path) {
        xmPathList->duplicatePath(path, mIndices[path] / 4, mStage);
    }

    xmPathList->clearOldPaths(mStage);

    for (unsigned path = 0; path < newPathCount; ++path) {
        xmPathList->getWriteAccessToNextBit(path, mStage);
        xmPathList->NextMetric(path) = mMetrics[path];
        fipv* LlrSource = xmPathList->NextLlr(path, mStage);
        union {
            fipv* BitDestination;
            char* cBitDestination;
        };
        BitDestination = xmPathList->NextBit(path, mStage);
        for (unsigned i = 0; i < mVecCount; ++i) {
            fi_store(BitDestination + i, fi_load(LlrSource + i));
        }

        for (unsigned i = 0; i < mBitFlipCount[mIndices[path]]; ++i) {
            unsigned index = mBitFlipHints[mIndices[path]][i];
            cBitDestination[index] = ~cBitDestination[index];
        }
    }

    xmPathList->switchToNext();
}


void RepetitionDecoder::decode()
{
    const fipv zero = fi_setzero();
    fipv llr, expanded;
    fipv vPos, vNeg, vZero, vOne, vResult;
    long metric;
    unsigned pathCount = xmPathList->PathCount();
    union {
        fipv* vLlr;
        char* cLlr;
    };

    for (unsigned path = 0; path < pathCount; ++path) {
        metric = xmPathList->Metric(path);
        vZero = fi_setzero();
        vOne = fi_setzero();
        vResult = fi_setzero();

        vLlr = xmPathList->Llr(path, mStage);
        for (unsigned i = mBlockLength; i < BYTESPERVECTOR; ++i) {
            cLlr[i] = 0;
        }

        for (unsigned i = 0; i < mVecCount; ++i) {
            llr = fi_load(vLlr + i);
            vResult = fi_adds_epi8(vResult, llr);

            vPos = fi_max_epi8(llr, zero);
            vNeg = fi_min_epi8(llr, zero);

            for (unsigned group = 0; group < BYTESPERVECTOR / 4; ++group) {
                expanded = llrExpandToLong(vNeg, group);
                vZero = fi_add_epi64(vZero, expanded);

                expanded = llrExpandToLong(vPos, group);
                vOne = fi_add_epi64(vOne, expanded);
            }
        }

        char result = std::max(reduce_adds_epi8(vResult), (char)-127);

        if (result < 0) {
            mResults[path * 2] = ~result;
            mResults[path * 2 + 1] = result;
        } else {
            mResults[path * 2] = result;
            mResults[path * 2 + 1] = ~result;
        }

        mMetrics[path * 2] = metric + reduce_add_epi64(vZero);
        mMetrics[path * 2 + 1] = metric - reduce_add_epi64(vOne);
    }
    unsigned newPathCount = std::min(pathCount * 2, xmPathList->PathLimit());
    xmPathList->setNextPathCount(newPathCount);
    simplePartialSortDescending(mIndices, mMetrics, newPathCount, pathCount * 2);

    for (unsigned path = 0; path < newPathCount; ++path) {
        xmPathList->duplicatePath(path, mIndices[path] / 2, mStage);
    }

    xmPathList->clearOldPaths(mStage);

    for (unsigned path = 0; path < newPathCount; ++path) {
        xmPathList->getWriteAccessToNextBit(path, mStage);
        xmPathList->NextMetric(path) = mMetrics[path];
        fipv* BitDestination = xmPathList->NextBit(path, mStage);
        fipv bits = fi_set1_epi8(mResults[mIndices[path]]);
        for (unsigned i = 0; i < mVecCount; ++i) {
            fi_store(BitDestination + i, bits);
        }
    }

    xmPathList->switchToNext();
}

void SpcDecoder::decode()
{
    const fipv absCorrector = fi_set1_epi8(-127);
    unsigned pathCount = xmPathList->PathCount();
    Block<fipv>* block = xmDataPool->allocate(std::max(mVecCount, 2U));
    union {
#ifdef __AVX2__
        __m256i* vTempBlock;
        __m128i* wTempBlock;
#else
        __m128i* vTempBlock;
#endif
        char* cTempBlock;
        long* lTempBlock;
    };
    union {
        fipv* vLlrSource;
        char* cLlrSource;
    };
    fipv vParity;

    vTempBlock = block->data;

    for (unsigned path = 0; path < pathCount; ++path) {
        vParity = fi_setzero();
        long metric = xmPathList->Metric(path);
        vLlrSource = xmPathList->Llr(path, mStage);

        for (unsigned i = mBlockLength; i < BYTESPERVECTOR; ++i) {
            cLlrSource[i] = 127;
        }

        for (unsigned i = 0; i < mVecCount; ++i) {
            fipv Llr = fi_load(vLlrSource + i);
            vParity = fi_xor(Llr, vParity);
            Llr = fi_abs_epi8(fi_max_epi8(Llr, absCorrector));
            fi_store(vTempBlock + i, Llr);
        }
        findWeakLlrs(mIndices, cTempBlock, mBlockLength, 4);

        unsigned char cParity = reduce_xor(vParity) & 0x80U;
        long weakest = 0;
#ifdef __AVX2__
        _mm256_store_si256(vTempBlock, _mm256_cvtepi8_epi64(_mm_load_si128(wTempBlock)));
#else
        { // To be optimized
            char t0 = cTempBlock[0];
            char t1 = cTempBlock[1];
            char t2 = cTempBlock[2];
            char t3 = cTempBlock[3];

            lTempBlock[0] = t0;
            lTempBlock[1] = t1;
            lTempBlock[2] = t2;
            lTempBlock[3] = t3;
        }
#endif

        if (cParity) {
            metric -= lTempBlock[0];
            mBitFlipCount[path * 8] = 1;
            mBitFlipHints[path * 8][0] = mIndices[0];
            mBitFlipCount[path * 8 + 1] = 0;
            mBitFlipCount[path * 8 + 2] = 0;
            mBitFlipCount[path * 8 + 3] = 0;
            mBitFlipCount[path * 8 + 4] = 1;
            mBitFlipHints[path * 8 + 4][0] = mIndices[0];
            mBitFlipCount[path * 8 + 5] = 1;
            mBitFlipHints[path * 8 + 5][0] = mIndices[0];
            mBitFlipCount[path * 8 + 6] = 1;
            mBitFlipHints[path * 8 + 6][0] = mIndices[0];
            mBitFlipCount[path * 8 + 7] = 0;
        } else {
            mBitFlipCount[path * 8] = 0;
            mBitFlipCount[path * 8 + 1] = 1;
            mBitFlipHints[path * 8 + 1][0] = mIndices[0];
            mBitFlipCount[path * 8 + 2] = 1;
            mBitFlipHints[path * 8 + 2][0] = mIndices[0];
            mBitFlipCount[path * 8 + 3] = 1;
            mBitFlipHints[path * 8 + 3][0] = mIndices[0];
            mBitFlipCount[path * 8 + 4] = 0;
            mBitFlipCount[path * 8 + 5] = 0;
            mBitFlipCount[path * 8 + 6] = 0;
            mBitFlipCount[path * 8 + 7] = 1;
            mBitFlipHints[path * 8 + 7][0] = mIndices[0];
            weakest = lTempBlock[0];
        }

        mMetrics[path * 8] = metric;
        mMetrics[path * 8 + 1] = metric - weakest - lTempBlock[1];
        mMetrics[path * 8 + 2] = metric - weakest - lTempBlock[2];
        mMetrics[path * 8 + 3] = metric - weakest - lTempBlock[3];
        mMetrics[path * 8 + 4] = metric - lTempBlock[1] - lTempBlock[2];
        mMetrics[path * 8 + 5] = metric - lTempBlock[1] - lTempBlock[3];
        mMetrics[path * 8 + 6] = metric - lTempBlock[2] - lTempBlock[3];
        mMetrics[path * 8 + 7] =
            metric - weakest - lTempBlock[1] - lTempBlock[2] - lTempBlock[3];

        mBitFlipHints[path * 8 + 1][mBitFlipCount[path * 8 + 1]++] = mIndices[1];
        mBitFlipHints[path * 8 + 2][mBitFlipCount[path * 8 + 2]++] = mIndices[2];
        mBitFlipHints[path * 8 + 3][mBitFlipCount[path * 8 + 3]++] = mIndices[3];
        mBitFlipHints[path * 8 + 4][mBitFlipCount[path * 8 + 4]++] = mIndices[1];
        mBitFlipHints[path * 8 + 4][mBitFlipCount[path * 8 + 4]++] = mIndices[2];
        mBitFlipHints[path * 8 + 5][mBitFlipCount[path * 8 + 5]++] = mIndices[1];
        mBitFlipHints[path * 8 + 5][mBitFlipCount[path * 8 + 5]++] = mIndices[3];
        mBitFlipHints[path * 8 + 6][mBitFlipCount[path * 8 + 6]++] = mIndices[2];
        mBitFlipHints[path * 8 + 6][mBitFlipCount[path * 8 + 6]++] = mIndices[3];
        mBitFlipHints[path * 8 + 7][mBitFlipCount[path * 8 + 7]++] = mIndices[1];
        mBitFlipHints[path * 8 + 7][mBitFlipCount[path * 8 + 7]++] = mIndices[2];
        mBitFlipHints[path * 8 + 7][mBitFlipCount[path * 8 + 7]++] = mIndices[3];
    }
    xmDataPool->release(block);

    unsigned newPathCount = std::min(pathCount * 8, xmPathList->PathLimit());
    xmPathList->setNextPathCount(newPathCount);
    simplePartialSortDescending(mIndices, mMetrics, newPathCount, pathCount * 8);

    for (unsigned path = 0; path < newPathCount; ++path) {
        xmPathList->duplicatePath(path, mIndices[path] / 8, mStage);
    }

    xmPathList->clearOldPaths(mStage);

    for (unsigned path = 0; path < newPathCount; ++path) {
        xmPathList->getWriteAccessToNextBit(path, mStage);
        xmPathList->NextMetric(path) = mMetrics[path];
        fipv* LlrSource = xmPathList->NextLlr(path, mStage);
        union {
            fipv* BitDestination;
            char* cBitDestination;
        };
        BitDestination = xmPathList->NextBit(path, mStage);
        for (unsigned i = 0; i < mVecCount; ++i) {
            fi_store(BitDestination + i, fi_load(LlrSource + i));
        }

        for (unsigned i = 0; i < mBitFlipCount[mIndices[path]]; ++i) {
            unsigned index = mBitFlipHints[mIndices[path]][i];
            cBitDestination[index] = ~cBitDestination[index];
        }
    }

    xmPathList->switchToNext();
}


Node* createDecoder(const std::vector<unsigned>& frozenBits, Node* parent)
{
    size_t blockLength = parent->blockLength();
    size_t frozenBitCount = frozenBits.size();

    if (frozenBitCount == blockLength) {
        return new RateZeroDecoder(parent);
    }

    if (frozenBitCount == 0) {
        return new RateOneDecoder(parent);
    }

    if (frozenBitCount == blockLength - 1) {
        return new RepetitionDecoder(parent);
    }

    if (frozenBitCount == 1) {
        return new SpcDecoder(parent);
    }

    if (blockLength <= BYTESPERVECTOR) {
        return new ShortRateRNode(frozenBits, parent);
    } else {
        return new RateRNode(frozenBits, parent);
    }
}

} // namespace SclFip

SclFipChar::SclFipChar(size_t blockLength,
                       size_t listSize,
                       const std::vector<unsigned>& frozenBits)
    : mListSize(listSize)
{
    initialize(blockLength, frozenBits);
}

SclFipChar::~SclFipChar() { clear(); }

void SclFipChar::clear()
{
    delete mEncoder;
    delete mRootNode;
    delete mNodeBase;
    delete mPathList;
    delete mDataPool;
}

void SclFipChar::initialize(size_t blockLength, const std::vector<unsigned>& frozenBits)
{
    if (blockLength == mBlockLength && frozenBits == mFrozenBits) {
        return;
    }
    if (mBlockLength != 0) {
        clear();
    }
    mBlockLength = blockLength;
    mFrozenBits.clear();
    mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
    mEncoder = new PolarCode::Encoding::ButterflyFipPacked(mBlockLength, mFrozenBits);
    mEncoder->setSystematic(false);
    mDataPool = new SclFip::datapool_t();
    mPathList =
        new SclFip::PathList(mListSize, __builtin_ctz(mBlockLength) + 1, mDataPool);
    mNodeBase = new SclFip::Node(mBlockLength, mListSize, mDataPool, mPathList);
    mRootNode = SclFip::createDecoder(frozenBits, mNodeBase);
    mLlrContainer = new CharContainer(mBlockLength);
    mBitContainer = new CharContainer(mBlockLength, frozenBits);
    mOutputContainer = new unsigned char[(mBlockLength - frozenBits.size() + 7) / 8];
}

bool SclFipChar::decode()
{
    makeInitialPathList();

    mRootNode->decode();

    return extractBestPath();
}

void SclFipChar::makeInitialPathList()
{
    mPathList->clear();
    mPathList->setFirstPath(dynamic_cast<CharContainer*>(mLlrContainer)->data());
}

bool SclFipChar::extractBestPath()
{
    unsigned dataStage = __builtin_ctz(mBlockLength);
    unsigned byteLength = (mBlockLength - mFrozenBits.size() + 7) / 8;
    unsigned pathCount = mPathList->PathCount();
    bool decoderSuccess = false;
    if (mSystematic) {
        for (unsigned path = 0; path < pathCount; ++path) {
            mBitContainer->insertCharBits(mPathList->Bit(path, dataStage));
            mBitContainer->getPackedInformationBits(mOutputContainer);
            if (mErrorDetector->check(mOutputContainer, byteLength)) {
                decoderSuccess = true;
                break;
            }
        }
        // Fall back to ML path, if none of the candidates was free of errors
        if (!decoderSuccess) {
            mBitContainer->insertCharBits(mPathList->Bit(0, dataStage));
            mBitContainer->getPackedInformationBits(mOutputContainer);
        }
    } else {
        for (unsigned path = 0; path < pathCount; ++path) {
            mEncoder->setCharCodeword(mPathList->Bit(path, dataStage));
            mEncoder->encode();
            mEncoder->getInformation(mOutputContainer);
            if (mErrorDetector->check(mOutputContainer, byteLength)) {
                decoderSuccess = true;
                break;
            }
        }
        // Fall back to ML path, if none of the candidates was free of errors
        if (!decoderSuccess) {
            mEncoder->setCharCodeword(mPathList->Bit(0, dataStage));
            mEncoder->encode();
            mEncoder->getInformation(mOutputContainer);
        }
    }
    mPathList->clear(); // Clean up
    return decoderSuccess;
}

} // namespace Decoding
} // namespace PolarCode
