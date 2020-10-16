/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "mathtest.h"
#include <signalprocessing/avx_mathfun.h>

CPPUNIT_TEST_SUITE_REGISTRATION(MathTest);

bool testVectors(__m256 a, __m256 b)
{
    __m256 result = _mm256_cmp_ps(a, b, _CMP_EQ_OQ);
    for (int i = 0; i < 8; ++i) {
        if (result[i] == 0) {
            std::cout << a[i] << "!=" << b[i] << " (" << (a[i] - b[i]) << ")"
                      << std::endl;
            return false;
        }
    }
    return true;
}

void MathTest::setUp() {}

void MathTest::tearDown() {}

void MathTest::test()
{
    __m256 input, output, expected;

    { // Testing log256_ps()
        input = _mm256_set_ps(
            1.0f, 10.0f, 100.0f, 1000.0f, 10000.0f, 100000.0f, 1000000.0f, 10000000.0f);
        expected = _mm256_set_ps(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f);

        output = log256_ps(input) / log256_ps(_mm256_set1_ps(10.0f));
        CPPUNIT_ASSERT(testVectors(output, expected));
    }

    { // Testing exp256_ps()
        input = _mm256_set_ps(1.0f, 2.0f, 3.0f, 4.0f, 20.0f, 48.0f, 64.0f, 32.0f);
        expected = _mm256_set_ps(1.0f, 2.0f, 3.0f, 4.0f, 20.0f, 48.0f, 64.0f, 32.0f);

        output = log256_ps(input);
        output = exp256_ps(output);

        CPPUNIT_ASSERT(testVectors(output, expected));
    }
}
