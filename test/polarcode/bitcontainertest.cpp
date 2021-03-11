/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze, 2021 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "bitcontainertest.h"
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <numeric>

CPPUNIT_TEST_SUITE_REGISTRATION(BitContainerTest);

void BitContainerTest::setUp()
{
    mTestData = "TestData";
    mFrozenBits = { 0, 1, 2, 3, 4, 5, 6, 7 }; // Freeze first byte

    floatContainer =
        std::make_unique<PolarCode::FloatContainer>(mTestData.size() * 8, mFrozenBits);
    charContainer =
        std::make_unique<PolarCode::CharContainer>(mTestData.size() * 8, mFrozenBits);
    packedContainer =
        std::make_unique<PolarCode::PackedContainer>(mTestData.size() * 8, mFrozenBits);

    control.resize(mTestData.size());
}

void BitContainerTest::tearDown() {}

void BitContainerTest::testFloatContainer()
{
    memset(control.data(), 0, mTestData.size());
    floatContainer->insertPackedBits(mTestData.data());
    floatContainer->getPackedBits(control.data());

    CPPUNIT_ASSERT(memcmp(mTestData.data(), control.data(), mTestData.size()) == 0);
}

void BitContainerTest::testFloatContainerWithFrozenBits()
{
    memset(control.data(), 0, mTestData.size());
    floatContainer->insertPackedInformationBits(mTestData.data());
    floatContainer->getPackedInformationBits(control.data());

    CPPUNIT_ASSERT(memcmp(mTestData.data(), control.data(), mTestData.size() - 1) == 0);
}

void BitContainerTest::testCharContainer()
{
    memset(control.data(), 0, mTestData.size());
    charContainer->insertPackedBits(mTestData.data());
    charContainer->getPackedBits(control.data());

    CPPUNIT_ASSERT(memcmp(mTestData.data(), control.data(), mTestData.size()) == 0);
}

void BitContainerTest::testCharContainerWithFrozenBits()
{
    memset(control.data(), 0, mTestData.size());
    charContainer->insertPackedInformationBits(mTestData.data());
    charContainer->getPackedInformationBits(control.data());

    CPPUNIT_ASSERT(memcmp(mTestData.data(), control.data(), mTestData.size() - 1) == 0);
}

void BitContainerTest::testCharContainerWithFloatInputLarge()
{
    const unsigned size = 64;
    const float scale = 4.0;
    std::vector<float> input(size);
    std::vector<char> expected(size);
    std::iota(input.begin(), input.end(), -4);
    for (unsigned i = 0; i < size; ++i) {
        input[i] = input[i] * scale + 0.3f;
        const float limited =
            std::max(float(INT8_MIN), std::min(float(INT8_MAX), std::round(input[i])));
        expected[i] = (char)limited;
    }

    std::vector<char> output(size);

    auto container = std::make_unique<PolarCode::CharContainer>(size);
    container->insertLlr(input.data());
    container->getSoftBits((void*)output.data());

    for (unsigned i = 0; i < size; ++i) {
        CPPUNIT_ASSERT_EQUAL(expected[i], output[i]);
    }
}

void BitContainerTest::testPackedContainer()
{
    memset(control.data(), 0, mTestData.size());
    packedContainer->insertPackedBits(mTestData.data());
    packedContainer->getPackedBits(control.data());

    CPPUNIT_ASSERT(memcmp(mTestData.data(), control.data(), mTestData.size()) == 0);
}

void BitContainerTest::testPackedContainerWithFrozenBits()
{
    memset(control.data(), 0, mTestData.size());
    packedContainer->insertPackedInformationBits(mTestData.data());
    packedContainer->getPackedInformationBits(control.data());

    CPPUNIT_ASSERT(memcmp(mTestData.data(), control.data(), mTestData.size() - 1) == 0);
}

void BitContainerTest::testPackedContainerOddSize()
{
    const unsigned nbyte = 4;
    std::vector<unsigned> frozen_bit_positions = { 0, 1, 2, 4, 5 };
    auto container =
        std::make_unique<PolarCode::PackedContainer>(nbyte * 8, frozen_bit_positions);
    std::vector<unsigned char> data(nbyte, 0xff);
    data[0] = 0x07;
    // std::vector<unsigned char> data = {
    //     0x07, 0xff,
    // };
    container->insertPackedInformationBits(data.data());
    std::vector<unsigned char> result(nbyte);
    container->getPackedBits(result.data());

    fmt::print("data:   ");
    for (const auto& b : data) {
        fmt::print("{:02x}\t", b);
    }
    fmt::print("\n");
    fmt::print("result: ");
    for (const auto& b : result) {
        fmt::print("{:02x}\t", b);
    }
    fmt::print("\n");
    // fmt::print("data:   {:b}\n", data);
    // fmt::print("result: {:b}\n", result);
}
