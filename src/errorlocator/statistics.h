/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PCERL_STATISTICS_H
#define PCERL_STATISTICS_H

#include <map>
#include <mutex>
#include <vector>

namespace SimulationErrorLocator {

struct StatisticsOutput {
    float min, max;
    float mean, dev;
    float sum;
};

class Statistics
{
    std::vector<float> mContainer;

public:
    Statistics();
    ~Statistics();

    void insert(float value);
    void clear();

    StatisticsOutput evaluate();

    void printContents();

    std::vector<float> valueList() { return mContainer; }
};

} // namespace SimulationErrorLocator

#endif
