/* -*- c++ -*- */
/*
 * Copyright 2020 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <polarcode/construction/betaexpansion.h>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace PolarCode {
namespace Construction {

// argsort is in an anonymous namespace to make it only available in this very file.
namespace {
template <typename T>
std::vector<size_t> argsort(const std::vector<T>& array)
{
    std::vector<size_t> indices(array.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&array](int left, int right) -> bool {
        // sort indices according to corresponding array element
        return array[left] < array[right];
    });

    return indices;
}
} // namespace

BetaExpansion::BetaExpansion() {}

BetaExpansion::BetaExpansion(size_t N, size_t K)
{
    setBlockLength(N);
    setInformationLength(K);
}

std::vector<unsigned> BetaExpansion::construct()
{
    if (mBlockLength < mInformationLength) {
        std::string error_msg =
            fmt::format("Invalid polar code({}, {})", mBlockLength, mInformationLength);
        throw std::invalid_argument(error_msg);
    }

    calculateChannelParameters();
    auto order_indices = argsort(mChannelParameters);

    const unsigned frozen_bit_length = mBlockLength - mInformationLength;
    std::vector<unsigned> frozenBits(order_indices.begin(),
                                     order_indices.begin() + frozen_bit_length);
    std::sort(frozenBits.begin(), frozenBits.end());
    return frozenBits;
}

void BetaExpansion::calculateChannelParameters()
{
    mChannelParameters.resize(mBlockLength);
    const unsigned block_power = static_cast<unsigned>(std::log2(mBlockLength));
    std::vector<double> weights(block_power);
    for (unsigned j = 0; j < block_power; j++) {
        weights[j] = std::pow(m_beta_value, static_cast<double>(j));
    }

    for (unsigned i = 0; i < mBlockLength; i++) {
        double weight = 0.0;
        for (unsigned j = 0; j < block_power; j++) {
            weight += weights[j] * ((i >> j) & 0x1);
        }
        mChannelParameters[i] = weight;
    }
}

} // namespace Construction
} // namespace PolarCode
