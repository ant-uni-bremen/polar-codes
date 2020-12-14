/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/construction/betaexpansion.h>
#include <polarcode/construction/bhattacharrya.h>
#include <polarcode/construction/constructor.h>
#include <polarcode/construction/fiveGList.h>
#include <algorithm>
#include <cmath>
#include <memory>
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

std::vector<unsigned> frozen_bits(const int blockLength,
                                  const int infoLength,
                                  const float designSnr,
                                  const std::string& constructor_type)
{
    auto ctype(constructor_type);
    std::transform(ctype.begin(), ctype.end(), ctype.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    std::unique_ptr<PolarCode::Construction::Constructor> constructor;
    if (ctype.find("be") != std::string::npos) {
        constructor = std::make_unique<PolarCode::Construction::BetaExpansion>(
            blockLength, infoLength, designSnr);
    } else if (ctype.find("5g") != std::string::npos) {
        constructor = std::make_unique<PolarCode::Construction::FiveGList>(
            blockLength, infoLength, designSnr);
    } else {
        constructor = std::make_unique<PolarCode::Construction::Bhattacharrya>(
            blockLength, infoLength, designSnr);
    }

    return constructor->construct();
}

} // namespace Construction
} // namespace PolarCode
