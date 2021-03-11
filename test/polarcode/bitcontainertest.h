/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_TEST_BITCONTAINER_H
#define PC_TEST_BITCONTAINER_H

#include <cppunit/extensions/HelperMacros.h>
#include <polarcode/bitcontainer.h>
#include <memory>
#include <vector>

class BitContainerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(BitContainerTest);
    CPPUNIT_TEST(testFloatContainer);
    CPPUNIT_TEST(testFloatContainerWithFrozenBits);
    CPPUNIT_TEST(testCharContainer);
    CPPUNIT_TEST(testCharContainerWithFrozenBits);
    CPPUNIT_TEST(testCharContainerWithFloatInputLarge);
    CPPUNIT_TEST(testPackedContainer);
    CPPUNIT_TEST(testPackedContainerWithFrozenBits);
    CPPUNIT_TEST(testPackedContainerOddSize);
    CPPUNIT_TEST_SUITE_END();

    std::unique_ptr<PolarCode::BitContainer> floatContainer;
    std::unique_ptr<PolarCode::BitContainer> charContainer;
    std::unique_ptr<PolarCode::BitContainer> packedContainer;
    std::vector<unsigned char> control;
    std::string mTestData;
    std::vector<unsigned> mFrozenBits;

public:
    void setUp();
    void tearDown();

    void testFloatContainer();
    void testFloatContainerWithFrozenBits();
    void testCharContainer();
    void testCharContainerWithFrozenBits();
    void testCharContainerWithFloatInputLarge();
    void testPackedContainer();
    void testPackedContainerWithFrozenBits();
    void testPackedContainerOddSize();
};

#endif // PC_TEST_BITCONTAINER_H
