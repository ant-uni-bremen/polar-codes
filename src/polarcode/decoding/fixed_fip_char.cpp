/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/decoding/fixed_fip_char.h>
#include <polarcode/encoding/butterfly_fip_packed.h>
#include <polarcode/errordetection/dummy.h>

namespace PolarCode {
namespace Decoding {

namespace FixedDecoding {

FixedDecoder::FixedDecoder() {}

FixedDecoder::~FixedDecoder() {}

} // namespace FixedDecoding


FixedChar::FixedChar(unsigned int scheme)
{
    mDecoder = FixedDecoding::createFixedDecoder(scheme);

    mBlockLength = codeRegistry[scheme].blockLength;
    mSystematic = codeRegistry[scheme].systematic;
    mFrozenBits.assign(codeRegistry[scheme].frozenBits.begin(),
                       codeRegistry[scheme].frozenBits.end());
    mEncoder = new Encoding::ButterflyFipPacked(mBlockLength, mFrozenBits);
    mEncoder->setSystematic(false);
    mLlrContainer = new CharContainer(mBlockLength, mFrozenBits);
    mBitContainer = new CharContainer(mBlockLength, mFrozenBits);
    mOutputContainer = new unsigned char[(codeRegistry[scheme].infoLength + 7) / 8];
}

FixedChar::~FixedChar()
{
    delete mEncoder;
    delete mDecoder;
}

bool FixedChar::decode()
{
    mDecoder->decode(reinterpret_cast<CharContainer*>(mLlrContainer)->data(),
                     reinterpret_cast<CharContainer*>(mBitContainer)->data());

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
