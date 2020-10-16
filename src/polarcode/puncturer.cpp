/* -*- c++ -*- */
/*
 * Copyright 2020 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/puncturer.h>
// #include <polarcode/avxconvenience.h>
// #include <immintrin.h>
#include <cassert>
// #include <cmath>
// #include <cstring>
#include <algorithm>
#include <bitset>
#include <iostream>
#include <numeric>
#include <stdexcept>

namespace PolarCode {


size_t round_up_power_of_two(size_t value)
{
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value++;
    return value;
}

std::vector<unsigned> inverse_set_difference(size_t blockLength,
                                             std::vector<unsigned> positions)
{
    std::vector<unsigned> v(blockLength);
    std::iota(v.begin(), v.end(), 0);

    std::vector<unsigned> diff;
    std::set_difference(v.begin(),
                        v.end(),
                        positions.begin(),
                        positions.end(),
                        std::inserter(diff, diff.begin()));
    return diff;
}

Puncturer::Puncturer(const size_t blockLength,
                     const std::vector<unsigned> frozenBitPositions)
    : mBlockLength(blockLength)
{
    mParentBlockLength = round_up_power_of_two(mBlockLength);
    auto numPuncturedPos = mParentBlockLength - mBlockLength;
    if (numPuncturedPos > frozenBitPositions.size()) {
        throw std::out_of_range(
            "Number of required puncturing positions exceeds frozen bit positions!");
    }
    std::vector<unsigned> puncturedPos(frozenBitPositions.begin(),
                                       frozenBitPositions.begin() + numPuncturedPos);
    assert(numPuncturedPos == puncturedPos.size());
    mOutputPositions = inverse_set_difference(mParentBlockLength, puncturedPos);
    assert(mOutputPositions.size() == mBlockLength);
}

Puncturer::~Puncturer() {}

void Puncturer::puncturePacked(unsigned char* pOutput, const unsigned char* pInput)
{
    assert(mParentBlockLength % 8 == 0);
    assert(mBlockLength % 8 == 0);
    assert(sizeof(unsigned char) == 1);
    size_t byteBlockLength = mBlockLength / 8;
    // unsigned outCounter = 0;
    auto outIterator = mOutputPositions.begin();

    for (unsigned b = 0; b < byteBlockLength; ++b) {
        std::bitset<8> outByte(0);
        for (unsigned i = 0; i < 8; ++i) {
            auto p = *outIterator++;
            unsigned bytepos = p / 8;
            std::bitset<8> inByte(pInput[bytepos]);
            outByte[7 - i] = inByte[7 - (p % 8)];
        }
        *pOutput++ = (unsigned char)outByte.to_ulong();
    }
}


} // namespace PolarCode
