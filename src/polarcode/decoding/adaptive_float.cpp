/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/decoding/adaptive_float.h>
#include <iostream>
namespace PolarCode {
namespace Decoding {


AdaptiveFloat::AdaptiveFloat(size_t blockLength,
                             size_t listSize,
                             const std::vector<unsigned>& frozenBits)
    : mListSize(listSize)
{
    mBlockLength = blockLength;
    mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
    mExternalContainers = true;
    mFastDecoder = std::make_unique<FastSscAvxFloat>(mBlockLength, mFrozenBits);
    mListDecoder = std::make_unique<SclAvxFloat>(mBlockLength, mListSize, mFrozenBits);
}

AdaptiveFloat::~AdaptiveFloat()
{
    mOutputContainer = nullptr;
    mBitContainer = nullptr;
}

bool AdaptiveFloat::decode()
{
    bool success = mFastDecoder->decode();
    mOutputContainer = mFastDecoder->packedOutput();
    mBitContainer = mFastDecoder->outputContainer();

    if (!success && mListSize > 1) {
        success = mListDecoder->decode();
        mOutputContainer = mListDecoder->packedOutput();
        mBitContainer = mListDecoder->outputContainer();
    }
    return success;
}

void AdaptiveFloat::setSystematic(bool sys)
{
    mFastDecoder->setSystematic(sys);
    mListDecoder->setSystematic(sys);
}

void AdaptiveFloat::setErrorDetection(ErrorDetection::Detector* pDetector)
{
    mFastDecoder->setErrorDetection(pDetector);
    mListDecoder->setErrorDetection(pDetector);
}

void AdaptiveFloat::setSignal(const float* pLlr)
{
    mFastDecoder->setSignal(pLlr);
    mListDecoder->setSignal(pLlr);
}


} // namespace Decoding
} // namespace PolarCode
