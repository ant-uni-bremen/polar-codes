/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/encoding/butterfly_fip.h>
#include <polarcode/encoding/butterfly_fip_packed.h>
#include <cmath>
#include <iostream>


namespace PolarCode {
namespace Encoding {


ButterflyFipPacked::ButterflyFipPacked() {}

ButterflyFipPacked::ButterflyFipPacked(size_t blockLength)
{
    initialize(blockLength, {});
}

ButterflyFipPacked::ButterflyFipPacked(size_t blockLength,
                                       const std::vector<unsigned>& frozenBits)
{
    initialize(blockLength, frozenBits);
}

ButterflyFipPacked::~ButterflyFipPacked() {}

void ButterflyFipPacked::initialize(size_t blockLength,
                                    const std::vector<unsigned>& frozenBits)
{
    mBlockLength = blockLength;
    mFrozenBits.assign(frozenBits.begin(), frozenBits.end());

    if (mBitContainer != nullptr)
        delete mBitContainer;
    mBitContainer = new PackedContainer(mBlockLength, mFrozenBits);
}

void ButterflyFipPacked::encode()
{
    if (!mCodewordReady) {
        mErrorDetector->generate(xmInputData, informationByteSize());
        mBitContainer->insertPackedInformationBits(xmInputData);
    }

    transform();

    if (mSystematic) {
        mBitContainer->resetFrozenBits();
        transform();
    }
    mCodewordReady = false;
}

void ButterflyFipPacked::transform()
{
    fipv* vBit =
        reinterpret_cast<fipv*>(dynamic_cast<PackedContainer*>(mBitContainer)->data());
    int n = __builtin_ctz(mBlockLength); // log2() on powers of 2

    for (int stage = 0; stage < n; ++stage) {
        ButterflyFipPackedTransform(vBit, mBlockLength, stage);
    }
}


} // namespace Encoding
} // namespace PolarCode
