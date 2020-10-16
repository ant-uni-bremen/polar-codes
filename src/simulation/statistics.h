/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PCSIM_STATISTICS_H
#define PCSIM_STATISTICS_H

#include <vector>

namespace Simulation {

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

    std::vector<float> valueList() { return mContainer; };
};

} // namespace Simulation

#endif
