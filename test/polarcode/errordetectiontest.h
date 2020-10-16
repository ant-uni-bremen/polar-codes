/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_TEST_ERRORDETECTION_H
#define PC_TEST_ERRORDETECTION_H

#include <cppunit/extensions/HelperMacros.h>

#include <polarcode/errordetection/cmac.h>
#include <polarcode/errordetection/crc32.h>
#include <polarcode/errordetection/crc8.h>
#include <polarcode/errordetection/dummy.h>

class ErrorDetectionTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(ErrorDetectionTest);
    CPPUNIT_TEST(testDummy);
    CPPUNIT_TEST(testCrc8);
    CPPUNIT_TEST(testCrc32);
    CPPUNIT_TEST(testCmac);
    CPPUNIT_TEST_SUITE_END();

    PolarCode::ErrorDetection::Detector *mDummy, *mCrc8, *mCrc32, *mCmac;

    size_t mDataLength;
    char* mData;
    unsigned char* mTestInput;

    void generalTest(PolarCode::ErrorDetection::Detector* detector);

public:
    void setUp();
    void tearDown();

    void testDummy();
    void testCrc8();
    void testCrc32();
    void testCmac();
};

#endif // PC_TEST_ERRORDETECTION_H
