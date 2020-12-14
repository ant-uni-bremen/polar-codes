/* -*- c++ -*- */
/*
 * Copyright 2020 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <polarcode/construction/fiveGList.h>
#include <stdexcept>

namespace PolarCode {
namespace Construction {

FiveGList::FiveGList() {}

FiveGList::FiveGList(size_t N, size_t K)
{
    if(N > 1024){
        throw std::invalid_argument("5G standard does not allow for block size N > 1024!");
    }
    setBlockLength(N);
    setInformationLength(K);
}

std::vector<unsigned> FiveGList::construct()
{
    if (mBlockLength < mInformationLength) {
        std::string error_msg =
            fmt::format("Invalid polar code({}, {})", mBlockLength, mInformationLength);
        throw std::invalid_argument(error_msg);
    }

    const unsigned frozen_bit_length = mBlockLength - mInformationLength;
    std::vector<unsigned> frozenBits(RELIABILITY_TABLE.begin(),
                                     RELIABILITY_TABLE.begin() + frozen_bit_length);
    std::sort(frozenBits.begin(), frozenBits.end());
    return frozenBits;
}

} // namespace Construction
} // namespace PolarCode
