/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "siformat.h"

#include <cmath>
#include <sstream>

std::string toPrecision(double num, int n)
{
    // https://stackoverflow.com/questions/202302/rounding-to-an-arbitrary-number-of-significant-digits

    if (num == 0) {
        return "0";
    }

    double d = std::ceil(std::log10(num < 0 ? -num : num));
    int power = n - (int)d;
    double magnitude = std::pow(10., power);
    long shifted = ::round(num * magnitude);

    std::ostringstream oss;
    oss << shifted / magnitude;
    return oss.str();
}

std::string siFormat(float x)
{
    static const std::string less[] = { "m", "µ", "n", "p", "f", "a" };
    static const std::string more[] = { "k", "M", "G", "T", "P", "E" };
    std::string result = "";
    int counter = -1;

    if (x < 1.0) {
        do {
            x *= 1000.0;
            ++counter;
        } while (x < 1.0 && counter < 5);
        result = toPrecision(x, 4);
        result += " ";
        result += less[counter];
    } else if (x >= 1000.0) {
        do {
            x /= 1000.0;
            ++counter;
        } while (x >= 1000.0 && counter < 5);
        result = toPrecision(x, 4);
        result += " ";
        result += more[counter];
    }
    return result;
}
