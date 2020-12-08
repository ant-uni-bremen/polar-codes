/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_TEST_DECODING_H
#define PC_TEST_DECODING_H

#include <cppunit/extensions/HelperMacros.h>
#include <polarcode/decoding/decoder.h>

class DecodingTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(DecodingTest);
    CPPUNIT_TEST(testSpecialDecoders);
    CPPUNIT_TEST(testGeneralDecodingFunctionsAvx);
#ifdef __AVX2__
    CPPUNIT_TEST(testGeneralDecodingFunctionsAvx2);
#else
    CPPUNIT_TEST(testGeneralDecodingFunctionsSse);
#endif
    CPPUNIT_TEST(testFipShort);
    CPPUNIT_TEST(testPerformance);
    CPPUNIT_TEST(testListDecoder);
    CPPUNIT_TEST(testTemplatized);
    CPPUNIT_TEST(testScan);
    CPPUNIT_TEST(testRepetitionCodeFloat);
    CPPUNIT_TEST(testDoubleRepetitionCodeFloat);
    CPPUNIT_TEST(testRepetitionCodeFipLong);
    CPPUNIT_TEST(testDoubleRepetitionCodeFipLong);
    CPPUNIT_TEST(testSPCCodeFloat);
    CPPUNIT_TEST(testDoubleSPCCodeFloat);
    CPPUNIT_TEST(testTypeFiveDecoder);
    CPPUNIT_TEST(testRepRateOneDecoderShort8);

    CPPUNIT_TEST_SUITE_END();

    PolarCode::Decoding::Decoder* mDecoder;

public:
    void setUp();
    void tearDown();

    void testSpecialDecoders();
    void testGeneralDecodingFunctionsAvx();
#ifdef __AVX2__
    void testGeneralDecodingFunctionsAvx2();
#else
    void testGeneralDecodingFunctionsSse();
#endif
    void testFipShort();
    void testPerformance();
    void testListDecoder();
    void testTemplatized();
    void testScan();
    void testRepetitionCodeFloat();
    void runRepetitionCodeFloat(const size_t block_length);
    void testDoubleRepetitionCodeFloat();
    void runDoubleRepetitionCodeFloat(const size_t block_length);

    void testSPCCodeFloat();
    void runSPCCodeFloat(const size_t block_length);
    void testDoubleSPCCodeFloat();
    void runDoubleSPCCodeFloat(const size_t block_length);

    void testRepetitionCodeFipLong();
    void runRepetitionCodeFipLong(const size_t block_length);
    void testDoubleRepetitionCodeFipLong();
    void runDoubleRepetitionCodeFipLong(const size_t block_length);

    void testTypeFiveDecoder();
    void runTypeFiveDecoder(const size_t block_length);

    void testRepRateOneDecoderShort8();

private:
    void showScanTestOutput(unsigned, float*);
    void fillRandom(float* vec, const unsigned length);
};

#endif // PC_TEST_DECODING_H
