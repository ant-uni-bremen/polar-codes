/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "constructiontest.h"
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <polarcode/construction/betaexpansion.h>
#include <polarcode/construction/bhattacharrya.h>
#include <stdexcept>

CPPUNIT_TEST_SUITE_REGISTRATION(ConstructionTest);

void ConstructionTest::setUp() {}

void ConstructionTest::tearDown() {}

void ConstructionTest::testBhattacharrya()
{
    std::vector<unsigned> output;
    std::vector<unsigned> expectedOutput({ 0, 1, 2, 4 });

    mConstructor = std::make_unique<PolarCode::Construction::Bhattacharrya>(8, 4);
    output = mConstructor->construct();
    CPPUNIT_ASSERT(output == expectedOutput);

    output.clear();

    expectedOutput = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                       16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 32,
                       33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 48, 49, 50, 52,
                       56, 64, 65, 66, 67, 68, 69, 70, 72, 73, 74, 76, 80, 81, 82, 96 };
    mConstructor = std::make_unique<PolarCode::Construction::Bhattacharrya>(128, 64);
    output = mConstructor->construct();
    CPPUNIT_ASSERT(output == expectedOutput);
}

void ConstructionTest::testBetaExpansion()
{
    std::vector<unsigned> output;
    mConstructor = std::make_unique<PolarCode::Construction::BetaExpansion>(4, 8);
    CPPUNIT_ASSERT_THROW(output = mConstructor->construct(), std::invalid_argument);

    mConstructor = std::make_unique<PolarCode::Construction::BetaExpansion>(8, 4);
    std::vector<unsigned> expectedOutput({ 0, 1, 2, 4 });
    output = mConstructor->construct();
    CPPUNIT_ASSERT(output == expectedOutput);

    mConstructor = std::make_unique<PolarCode::Construction::BetaExpansion>(64, 32);
    std::vector<unsigned> expectedOutput64({ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                             11, 12, 13, 14, 16, 17, 18, 19, 20, 21, 22,
                                             24, 25, 32, 33, 34, 35, 36, 37, 40, 48 });
    output = mConstructor->construct();
    CPPUNIT_ASSERT(output == expectedOutput64);

    mConstructor = std::make_unique<PolarCode::Construction::BetaExpansion>(256, 64);
    std::vector<unsigned>
        expectedOutput256(
            { 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
              15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
              30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
              45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
              60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,
              75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
              90,  91,  92,  93,  94,  96,  97,  98,  99,  100, 101, 102, 103, 104, 105,
              106, 107, 108, 109, 110, 112, 113, 114, 115, 116, 117, 118, 120, 121, 128,
              129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
              144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158,
              160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174,
              176, 177, 178, 179, 180, 184, 192, 193, 194, 195, 196, 197, 198, 199, 200,
              201, 202, 204, 208, 209, 210, 212, 216, 224, 225, 226, 228 });

    output = mConstructor->construct();
    CPPUNIT_ASSERT(output == expectedOutput256);
}
