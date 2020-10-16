/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/construction/bhattacharrya.h>
#include <polarcode/construction/constructor.h>
#include <cmath>
#include <stdexcept>

namespace PolarCode {
namespace Construction {

Constructor::Constructor() : mBlockLength(0), mInformationLength(0), mDesignSnr(0.0) {}

Constructor::~Constructor() {}

void Constructor::setBlockLength(size_t newBlockLength)
{
    size_t testLength = 1 << static_cast<size_t>(log2(newBlockLength));
    if (testLength != newBlockLength) {
        throw std::invalid_argument("new blockLength is not a power of 2!");
    }
    mBlockLength = newBlockLength;
}

void Constructor::setInformationLength(size_t newInformationLength)
{
    mInformationLength = newInformationLength;
}

void Constructor::setDesignSnr(float designSnr) { mDesignSnr = designSnr; }

std::vector<unsigned>
frozen_bits(const int blockLength, const int infoLength, const float designSnr)
{
    auto constructor =
        new PolarCode::Construction::Bhattacharrya(blockLength, infoLength, designSnr);
    auto indices = constructor->construct();
    delete constructor;
    return indices;
}

} // namespace Construction
} // namespace PolarCode
