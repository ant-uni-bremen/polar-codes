/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "statistics.h"

#include <cmath>
#include <iostream>

namespace Simulation {

Statistics::Statistics() {}

Statistics::~Statistics() {}

void Statistics::insert(float value) { mContainer.push_back(value); }

void Statistics::clear() { mContainer.clear(); }

StatisticsOutput Statistics::evaluate()
{
    StatisticsOutput ret;

    if (mContainer.empty()) {
        return { 0 };
    }

    unsigned size = mContainer.size();
    unsigned minIdx = 0, maxIdx = 0;
    float sum = 0.0, mean;

    // Find minimum, maximum and calculate mean
    for (unsigned i = 0; i < size; ++i) {
        sum += mContainer[i];
        if (mContainer[i] < mContainer[minIdx]) {
            minIdx = i;
        } else if (mContainer[i] > mContainer[maxIdx]) {
            maxIdx = i;
        }
    }
    mean = sum / size;

    ret.min = mContainer[minIdx];
    ret.max = mContainer[maxIdx];
    ret.mean = mean;
    ret.sum = sum;

    // Calculate deviation
    sum = 0.0;
    for (unsigned i = 0; i < size; ++i) {
        float tmp = mContainer[i] - mean;
        sum += tmp * tmp;
    }
    ret.dev = sqrt(sum / (size - 1));

    return ret;
}

void Statistics::printContents()
{
    for (float f : mContainer) {
        std::cout << f << std::endl;
    }
}

} // namespace Simulation
