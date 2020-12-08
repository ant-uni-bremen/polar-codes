/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "decodingtest.h"
#include "siformat.h"

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <polarcode/construction/bhattacharrya.h>
#include <polarcode/decoding/fastssc_avx_float.h>
#include <polarcode/decoding/fastssc_fip_char.h>
#include <polarcode/decoding/fastsscan_float.h>
#include <polarcode/decoding/fip_templates.txx>
#include <polarcode/decoding/scan.h>
#include <polarcode/decoding/scl_avx_float.h>
#include <polarcode/decoding/scl_fip_char.h>
#include <polarcode/decoding/templatized_float.h>
#include <chrono>
#include <cstdlib>
#include <random>

CPPUNIT_TEST_SUITE_REGISTRATION(DecodingTest);

void DecodingTest::setUp() {}

void DecodingTest::tearDown() {}

bool testShortVectors(const fipv& one, const fipv& two, const size_t length)
{
    return 0 == memcmp(&one, &two, length);
}

bool testVectors(const fipv& one, const fipv& two)
{
    return testShortVectors(one, two, BYTESPERVECTOR);
}

bool testBitVectors(const fipv& one, const fipv& two)
{
    const fipv sgnMask = fi_set1_epi8(-128);
    const fipv zero = fi_setzero();

    const fipv sone = fi_and(one, sgnMask);
    const fipv stwo = fi_and(two, sgnMask);
    const fipv test = fi_xor(sone, stwo);

    return testShortVectors(test, zero, BYTESPERVECTOR);
}

bool testShortVectors(const __m256& one, const __m256& two, const size_t length)
{
    return 0 == memcmp(&one, &two, length * 4);
}

bool testVectors(const __m256& one, const __m256& two)
{
    return testShortVectors(one, two, 8);
}

bool testBitVectors(const __m256& one, const __m256& two)
{
    const __m256 sgnMask = _mm256_set1_ps(-0.0f);
    const __m256 zero = _mm256_setzero_ps();

    const __m256 sone = _mm256_and_ps(one, sgnMask);
    const __m256 stwo = _mm256_and_ps(two, sgnMask);
    const __m256 test = _mm256_xor_ps(sone, stwo);

    return testShortVectors(zero, test, 8);
}

void DecodingTest::testRepetitionCodeFipLong()
{
    runRepetitionCodeFipLong(32);
    runRepetitionCodeFipLong(64);
}

void DecodingTest::runRepetitionCodeFipLong(const size_t block_length)
{
    std::vector<unsigned> frozen_bit_positions(block_length - 1);
    std::iota(frozen_bit_positions.begin(), frozen_bit_positions.end(), 0);
    auto decoder = std::make_unique<PolarCode::Decoding::FastSscFipChar>(
        block_length, frozen_bit_positions);
    decoder->setSystematic(false);

    char* signal = (char*)std::aligned_alloc(32, block_length * sizeof(char));
    char* output = (char*)std::aligned_alloc(32, block_length * sizeof(char));
    std::iota(signal, signal + block_length, (-1 * block_length / 2) - 1);
    decoder->setSignal(signal);
    decoder->decode();
    decoder->getSoftCodeword(output);

    int iresult = 0;
    for (unsigned i = 0; i < block_length; i++) {
        iresult += int(signal[i]);
    }

    const char result = char(std::min(127, std::max(-128, iresult)));
    fmt::print("testRepetitionCodeFipChar: block_length={}\n", block_length);
    for (unsigned i = 0; i < block_length; i++) {
        CPPUNIT_ASSERT_EQUAL(result, output[i]);
    }

    free(signal);
    free(output);
}

void DecodingTest::testDoubleRepetitionCodeFipLong()
{
    runDoubleRepetitionCodeFipLong(32);
    runDoubleRepetitionCodeFipLong(64);
}
void DecodingTest::runDoubleRepetitionCodeFipLong(const size_t block_length)
{

    std::vector<unsigned> frozen_bit_positions(block_length - 2);
    std::iota(frozen_bit_positions.begin(), frozen_bit_positions.end(), 0);
    auto decoder = std::make_unique<PolarCode::Decoding::FastSscFipChar>(
        block_length, frozen_bit_positions);
    decoder->setSystematic(false);

    char* signal = (char*)std::aligned_alloc(32, block_length * sizeof(char));
    char* output = (char*)std::aligned_alloc(32, block_length * sizeof(char));
    std::iota(signal, signal + block_length, (-1 * block_length / 2) - 1);
    decoder->setSignal(signal);
    decoder->decode();
    decoder->getSoftCodeword(output);

    int iresult0 = 0;
    int iresult1 = 0;
    for (unsigned i = 0; i < block_length; i += 2) {
        iresult0 += int(signal[i]);
        iresult1 += int(signal[i + 1]);
    }
    const char result0 = char(std::min(127, std::max(-128, iresult0)));
    const char result1 = char(std::min(127, std::max(-128, iresult1)));

    fmt::print(
        "testDoubleRepetitionCodeFipChar: block_length={:>4d}\tr0={:>+3d}\tr1={:>+3d}\n",
        block_length,
        result0,
        result1);

    for (unsigned i = 0; i < block_length; i += 2) {
        CPPUNIT_ASSERT_EQUAL(result0, output[i]);
        CPPUNIT_ASSERT_EQUAL(result1, output[i + 1]);
    }

    free(signal);
    free(output);
}


void DecodingTest::runRepetitionCodeFloat(const size_t block_length)
{
    std::vector<unsigned> frozen_bit_positions(block_length - 1);
    std::iota(frozen_bit_positions.begin(), frozen_bit_positions.end(), 0);
    auto decoder = std::make_unique<PolarCode::Decoding::FastSscAvxFloat>(
        block_length, frozen_bit_positions);
    decoder->setSystematic(false);

    float* signal = (float*)std::aligned_alloc(32, block_length * sizeof(float));
    float* output = (float*)std::aligned_alloc(32, block_length * sizeof(float));
    std::iota(signal, signal + block_length, 0);
    decoder->setSignal(signal);
    decoder->decode();
    decoder->getSoftCodeword(output);

    const float result = std::accumulate(signal, signal + block_length, 0.0f);
    std::cout << "testRepetitionCode: block_length=" << block_length << std::endl;
    for (unsigned i = 0; i < block_length; i++) {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(result, output[i], 1e-7);
    }

    free(signal);
    free(output);
}

void DecodingTest::testRepetitionCodeFloat()
{
    size_t block_length = 2;
    runRepetitionCodeFloat(block_length);
    runRepetitionCodeFloat(block_length * 2);
    runRepetitionCodeFloat(block_length * 4);
    runRepetitionCodeFloat(block_length * 8);
    runRepetitionCodeFloat(block_length * 16);
    runRepetitionCodeFloat(block_length * 32);
    runRepetitionCodeFloat(block_length * 64);
}


void DecodingTest::runDoubleRepetitionCodeFloat(const size_t block_length)
{
    std::vector<unsigned> frozen_bit_positions(block_length - 2);
    std::iota(frozen_bit_positions.begin(), frozen_bit_positions.end(), 0);

    auto decoder = std::make_unique<PolarCode::Decoding::FastSscAvxFloat>(
        block_length, frozen_bit_positions);
    decoder->setSystematic(false);

    float* signal = (float*)std::aligned_alloc(32, block_length * sizeof(float));
    float* output = (float*)std::aligned_alloc(32, block_length * sizeof(float));
    std::iota(signal, signal + block_length, 0);
    decoder->setSignal(signal);
    decoder->decode();
    decoder->getSoftCodeword(output);

    float result0 = 0.0f;
    float result1 = 0.0f;
    for (unsigned i = 0; i < block_length; i += 2) {
        result0 += signal[i];
        result1 += signal[i + 1];
    }
    std::cout << "testDoubleRepetitionCode: block_length=" << block_length
              << "\teven=" << result0 << "\todd=" << result1 << std::endl;

    for (unsigned i = 0; i < block_length; i += 2) {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(result0, output[i], 1e-7);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(result1, output[i + 1], 1e-7);
    }

    free(signal);
    free(output);
}

void DecodingTest::testDoubleRepetitionCodeFloat()
{
    size_t block_length = 4;
    runDoubleRepetitionCodeFloat(block_length);
    runDoubleRepetitionCodeFloat(block_length * 2);
    runDoubleRepetitionCodeFloat(block_length * 4);
    runDoubleRepetitionCodeFloat(block_length * 8);
    runDoubleRepetitionCodeFloat(block_length * 16);
    runDoubleRepetitionCodeFloat(block_length * 32);
    runDoubleRepetitionCodeFloat(block_length * 64);
}


void DecodingTest::runSPCCodeFloat(const size_t block_length)
{
    std::vector<unsigned> frozen_bit_positions(1);
    std::iota(frozen_bit_positions.begin(), frozen_bit_positions.end(), 0);
    auto decoder = std::make_unique<PolarCode::Decoding::FastSscAvxFloat>(
        block_length, frozen_bit_positions);
    decoder->setSystematic(false);

    float* signal = (float*)std::aligned_alloc(32, block_length * sizeof(float));
    float* output = (float*)std::aligned_alloc(32, block_length * sizeof(float));

    std::iota(signal, signal + block_length, -2.9);
    std::vector<float> expected(signal, signal + block_length);
    expected[3] *= -1.0;
    decoder->setSignal(signal);
    decoder->decode();
    decoder->getSoftCodeword(output);

    fmt::print("testSPCCodeFloat: block_length={}\n", block_length);
    for (unsigned i = 0; i < block_length; i++) {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(expected[i], output[i], 1e-7);
    }

    free(signal);
    free(output);
}

void DecodingTest::testSPCCodeFloat()
{
    size_t block_length = 4;
    runSPCCodeFloat(block_length);
    runSPCCodeFloat(block_length * 2);
    runSPCCodeFloat(block_length * 4);
    runSPCCodeFloat(block_length * 8);
    runSPCCodeFloat(block_length * 16);
    runSPCCodeFloat(block_length * 32);
    runSPCCodeFloat(block_length * 64);
}

void DecodingTest::fillRandom(float* vec, const unsigned length)
{
    std::mt19937_64 generator;
    std::normal_distribution<float> dist(10, 20);
    for (unsigned i = 0; i < length; ++i) {
        vec[i] = dist(generator);
    }
}

void DecodingTest::runDoubleSPCCodeFloat(const size_t block_length)
{

    fmt::print("testDoubleSPCFloat: block_length={}\n", block_length);
    std::vector<unsigned> frozen_bit_positions(2);
    std::iota(frozen_bit_positions.begin(), frozen_bit_positions.end(), 0);
    auto decoder = std::make_unique<PolarCode::Decoding::FastSscAvxFloat>(
        block_length, frozen_bit_positions);
    decoder->setSystematic(false);

    float* signal = (float*)std::aligned_alloc(32, block_length * sizeof(float));
    float* output = (float*)std::aligned_alloc(32, block_length * sizeof(float));

    fillRandom(signal, block_length);

    std::vector<float> expected(block_length);
    PolarCode::Decoding::FastSscAvx::decode_double_spc(
        expected.data(), signal, block_length);

    // fmt::print("signal: {}\n", std::vector<float>(signal, signal + block_length));
    decoder->setSignal(signal);
    decoder->decode();
    decoder->getSoftCodeword(output);

    // fmt::print("output: {}\n", std::vector<float>(output, output + block_length));
    // fmt::print("expect: {}\n", expected);
    for (unsigned i = 0; i < block_length; i++) {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(expected[i], output[i], 1e-7);
    }

    free(signal);
    free(output);
}

void DecodingTest::testDoubleSPCCodeFloat()
{
    size_t block_length = 4;
    runDoubleSPCCodeFloat(block_length * 2);
    runDoubleSPCCodeFloat(block_length * 4);
    runDoubleSPCCodeFloat(block_length * 8);
    runDoubleSPCCodeFloat(block_length * 16);
}


void DecodingTest::runTypeFiveDecoder(const size_t block_length)
{
    fmt::print("testTypeFiveDecoder: block_length={}\n", block_length);
    std::vector<unsigned> frozen_bit_positions(block_length - 4);
    std::iota(frozen_bit_positions.begin(), frozen_bit_positions.end() - 1, 0);
    frozen_bit_positions[block_length - 5] = block_length - 4;
    fmt::print("positions: {}\n", frozen_bit_positions);
    auto decoder = std::make_unique<PolarCode::Decoding::FastSscAvxFloat>(
        block_length, frozen_bit_positions);
    decoder->setSystematic(false);

    float* signal = (float*)std::aligned_alloc(32, block_length * sizeof(float));
    float* output = (float*)std::aligned_alloc(32, block_length * sizeof(float));

    fillRandom(signal, block_length);

    std::vector<float> expected(block_length);
    PolarCode::Decoding::FastSscAvx::decode_type_five_generic(
        expected.data(), signal, block_length);

    decoder->setSignal(signal);
    decoder->decode();
    decoder->getSoftCodeword(output);

    for (unsigned i = 0; i < block_length; i++) {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(expected[i], output[i], 1e-7);
    }

    free(signal);
    free(output);
}

void DecodingTest::testTypeFiveDecoder()
{
    size_t block_length = 4;
    runTypeFiveDecoder(block_length * 2);
    runTypeFiveDecoder(block_length * 4);
    runTypeFiveDecoder(block_length * 8);
    runTypeFiveDecoder(block_length * 16);
}


void DecodingTest::testSpecialDecoders()
{
/*	__m256i llr, bits, expectedResult;

	// Rate-0
	bits = _mm256_set1_epi8(-1);
	expectedResult = _mm256_setzero_si256();
	PolarCode::Decoding::FastSscAvx2::RateZeroDecode(&llr, &bits, 32);
	bits = PolarCode::Decoding::FastSscAvx2::hardDecode(bits);
	CPPUNIT_ASSERT(testVectors(bits, expectedResult));

	// Rate-1
	llr            = _mm256_set_epi8(-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1);
	bits           = _mm256_set1_epi8(-1);
	expectedResult = _mm256_set_epi8(-128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	PolarCode::Decoding::FastSscAvx2::RateOneDecode(&llr, &bits, 32);
	bits = PolarCode::Decoding::FastSscAvx2::hardDecode(bits);
	CPPUNIT_ASSERT(testVectors(bits, expectedResult));

	// Repetition
	llr            = _mm256_set_epi8(-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1);
	bits           = _mm256_set1_epi8(-1);
	expectedResult = _mm256_set_epi8(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	PolarCode::Decoding::FastSscAvx2::RepetitionDecode(&llr, &bits, 32);
	bits = PolarCode::Decoding::FastSscAvx2::hardDecode(bits);
	CPPUNIT_ASSERT(testVectors(bits, expectedResult));

	// SPC
	llr            = _mm256_set_epi8(-1,1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1);
	bits           = _mm256_set1_epi8(-1);
	expectedResult = _mm256_set_epi8(-128,0,-128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-128,-128);
	PolarCode::Decoding::FastSscAvx2::SpcDecode(&llr, &bits, 32);
	bits = PolarCode::Decoding::FastSscAvx2::hardDecode(bits);
	CPPUNIT_ASSERT(testVectors(bits, expectedResult));

	llr            = _mm256_set_epi8(-1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	bits           = _mm256_set1_epi8(-1);
	expectedResult = _mm256_set_epi8(0,-128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	PolarCode::Decoding::FastSscAvx2::SpcDecode(&llr, &bits, 2);
	CPPUNIT_ASSERT(testShortVectors(bits, expectedResult, 2));

	llr            = _mm256_set_epi8(1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	bits           = _mm256_set1_epi8(-1);
	expectedResult = _mm256_set_epi8(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	PolarCode::Decoding::FastSscAvx2::SpcDecode(&llr, &bits, 2);
	CPPUNIT_ASSERT(testShortVectors(bits, expectedResult, 2));
*/}

void DecodingTest::testGeneralDecodingFunctionsAvx()
{
    union fv {
        __m256 v;
        float f[8];
    };

    fv llr[2], bits, child;
    fv expected;

    // F-function
    llr[0].v = _mm256_set_ps(0, 1, 2, 3, 4, 5, 6, -7);
    llr[1].v = _mm256_set_ps(1, 2, 3, -4, 5, 6, -7, 8);
    expected.v = _mm256_set_ps(0, 1, 2, -3, 4, 5, -6, -7);
    PolarCode::Decoding::FastSscAvx::F_function(&llr[0].f[0], &child.f[0], 8);
    CPPUNIT_ASSERT(testVectors(child.v, expected.v));

    // G-function
    llr[1].v = _mm256_set_ps(-1, 2, 3, -4, 5, 6, -7, 8);
    llr[0].v = _mm256_set_ps(0, 1, 2, 3, 4, 5, 6, -7);
    bits.v = _mm256_set_ps(0, 0, 0, -128, 0, 0, -128, -128);
    expected.v = _mm256_set_ps(-1, 3, 5, -7, 9, 11, -13, 15);
    PolarCode::Decoding::FastSscAvx::G_function(&llr[0].f[0], &child.f[0], &bits.f[0], 8);
    CPPUNIT_ASSERT(testVectors(child.v, expected.v));

    // Combine-function
    llr[0].v = _mm256_setr_ps(1, 1, 1, 1, 0, 0, 0, 0);
    llr[1].v = _mm256_setr_ps(-1, -1, -1, -1, 0, 0, 0, 0);
    expected.v = _mm256_setr_ps(-1, -1, -1, -1, -1, -1, -1, -1);
    PolarCode::Decoding::FastSscAvx::CombineBitsShort(
        &llr[0].f[0], &llr[1].f[0], &bits.f[0], 4);
    CPPUNIT_ASSERT(testBitVectors(bits.v, expected.v));
}

#ifdef __AVX2__

void DecodingTest::testGeneralDecodingFunctionsAvx2()
{
    __m256i llr[2], bits, child;
    __m256i expected;

    // F-function
    llr[0] = _mm256_set_epi8(0,
                             1,
                             2,
                             3,
                             4,
                             5,
                             6,
                             -7,
                             8,
                             9,
                             0,
                             1,
                             2,
                             3,
                             4,
                             5,
                             6,
                             7,
                             8,
                             9,
                             0,
                             1,
                             2,
                             3,
                             4,
                             5,
                             6,
                             7,
                             8,
                             9,
                             0,
                             1);
    llr[1] = _mm256_set_epi8(-1,
                             2,
                             3,
                             -4,
                             5,
                             6,
                             -7,
                             8,
                             9,
                             0,
                             1,
                             2,
                             3,
                             4,
                             5,
                             6,
                             7,
                             8,
                             9,
                             0,
                             1,
                             2,
                             3,
                             4,
                             5,
                             6,
                             7,
                             8,
                             9,
                             0,
                             1,
                             2);
    expected = _mm256_set_epi8(-1,
                               1,
                               2,
                               -3,
                               4,
                               5,
                               -6,
                               -7,
                               8,
                               1,
                               1,
                               1,
                               2,
                               3,
                               4,
                               5,
                               6,
                               7,
                               8,
                               1,
                               1,
                               1,
                               2,
                               3,
                               4,
                               5,
                               6,
                               7,
                               8,
                               1,
                               1,
                               1);
    PolarCode::Decoding::FastSscFip::F_function(llr, &child, 32);
    CPPUNIT_ASSERT(testVectors(child, expected));
    PolarCode::Decoding::FixedDecoding::F_function<32>(llr, &child);
    CPPUNIT_ASSERT(testVectors(child, expected));

    // G-function
    llr[1] = _mm256_set_epi8(-1,
                             2,
                             3,
                             -4,
                             5,
                             6,
                             -7,
                             8,
                             9,
                             0,
                             1,
                             2,
                             3,
                             4,
                             5,
                             6,
                             7,
                             8,
                             9,
                             0,
                             1,
                             2,
                             3,
                             4,
                             5,
                             6,
                             7,
                             8,
                             9,
                             0,
                             1,
                             2);
    llr[0] = _mm256_set_epi8(0,
                             1,
                             2,
                             3,
                             4,
                             5,
                             6,
                             -7,
                             8,
                             9,
                             0,
                             1,
                             2,
                             3,
                             4,
                             5,
                             6,
                             7,
                             8,
                             9,
                             0,
                             1,
                             2,
                             3,
                             4,
                             5,
                             6,
                             7,
                             8,
                             9,
                             0,
                             1);
    bits = _mm256_set_epi8(0,
                           0,
                           0,
                           -128,
                           0,
                           0,
                           -128,
                           -128,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0);
    expected = _mm256_set_epi8(-1,
                               3,
                               5,
                               -7,
                               9,
                               11,
                               -13,
                               15,
                               17,
                               9,
                               1,
                               3,
                               5,
                               7,
                               9,
                               11,
                               13,
                               15,
                               17,
                               9,
                               1,
                               3,
                               5,
                               7,
                               9,
                               11,
                               13,
                               15,
                               17,
                               9,
                               1,
                               3);
    PolarCode::Decoding::FastSscFip::G_function(llr, &child, &bits, 32);
    CPPUNIT_ASSERT(testVectors(child, expected));
    PolarCode::Decoding::FixedDecoding::G_function<32>(llr, &child, &bits);
    CPPUNIT_ASSERT(testVectors(child, expected));

    // Combine-function
    llr[0] = _mm256_setr_epi8(127,
                              127,
                              127,
                              127,
                              127,
                              127,
                              127,
                              127,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0);
    llr[1] = _mm256_setr_epi8(-128,
                              -128,
                              -128,
                              -128,
                              -128,
                              -128,
                              -128,
                              -128,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0);
    expected = _mm256_setr_epi8(-127,
                                -127,
                                -127,
                                -127,
                                -127,
                                -127,
                                -127,
                                -127,
                                -127,
                                -127,
                                -127,
                                -127,
                                -127,
                                -127,
                                -127,
                                -127,
                                1,
                                1,
                                1,
                                1,
                                1,
                                1,
                                1,
                                1,
                                1,
                                1,
                                1,
                                1,
                                1,
                                1,
                                1,
                                1);
    PolarCode::Decoding::FastSscFip::CombineBitsShort(llr, llr + 1, &bits, 8);
    CPPUNIT_ASSERT(testBitVectors(bits, expected));
    PolarCode::Decoding::FixedDecoding::CombineBits<8>(llr, llr + 1, &bits);
    CPPUNIT_ASSERT(testBitVectors(bits, expected));

    // Extended combine-test
    {
        union charVec {
            __m256i v;
            char c[32];
        };

        charVec llrLeft;
        charVec llrRight;
        charVec result;
        for (int left = -128; left < 0; left++) {
            for (int right = -128; right < 0; right++) {
                llrLeft.c[0] = static_cast<char>(left);
                llrRight.c[0] = static_cast<char>(right);
                PolarCode::Decoding::FastSscFip::CombineBitsShort(
                    &llrLeft.v, &llrRight.v, &result.v, 1);
                CPPUNIT_ASSERT(result.c[0] >= 0);
                PolarCode::Decoding::FixedDecoding::CombineBits<1>(
                    &llrLeft.v, &llrRight.v, &result.v);
                CPPUNIT_ASSERT(result.c[0] >= 0);
            }
        }
        for (int left = 0; left < 128; left++) {
            for (int right = 0; right < 128; right++) {
                llrLeft.c[0] = static_cast<char>(left);
                llrRight.c[0] = static_cast<char>(right);
                PolarCode::Decoding::FastSscFip::CombineBitsShort(
                    &llrLeft.v, &llrRight.v, &result.v, 1);
                CPPUNIT_ASSERT(result.c[0] >= 0);
                PolarCode::Decoding::FixedDecoding::CombineBits<1>(
                    &llrLeft.v, &llrRight.v, &result.v);
                CPPUNIT_ASSERT(result.c[0] >= 0);
            }
        }
        for (int left = -128; left < 0; left++) {
            for (int right = 0; right < 128; right++) {
                llrLeft.c[0] = static_cast<char>(left);
                llrRight.c[0] = static_cast<char>(right);
                PolarCode::Decoding::FastSscFip::CombineBitsShort(
                    &llrLeft.v, &llrRight.v, &result.v, 1);
                // std::cout << "Left: " << left << ", Right: " << right << ", Result: "
                // << (int)result.c[0] << std::endl;
                CPPUNIT_ASSERT(result.c[0] < 0);
                PolarCode::Decoding::FixedDecoding::CombineBits<1>(
                    &llrLeft.v, &llrRight.v, &result.v);
                CPPUNIT_ASSERT(result.c[0] < 0);
            }
        }
        for (int left = 0; left < 128; left++) {
            for (int right = -128; right < 0; right++) {
                llrLeft.c[0] = static_cast<char>(left);
                llrRight.c[0] = static_cast<char>(right);
                PolarCode::Decoding::FastSscFip::CombineBitsShort(
                    &llrLeft.v, &llrRight.v, &result.v, 1);
                CPPUNIT_ASSERT(result.c[0] < 0);
                PolarCode::Decoding::FixedDecoding::CombineBits<1>(
                    &llrLeft.v, &llrRight.v, &result.v);
                CPPUNIT_ASSERT(result.c[0] < 0);
            }
        }
    }
}

#else

void DecodingTest::testGeneralDecodingFunctionsSse()
{
    __m128i llr[2], bits, child;
    __m128i expected;

    // F-function
    llr[0] = _mm_set_epi8(0, 1, 2, 3, 4, 5, 6, -7, 8, 9, 0, 1, 2, 3, 4, 5);
    llr[1] = _mm_set_epi8(-1, 2, 3, -4, 5, 6, -7, 8, 9, 0, 1, 2, 3, 4, 5, 6);
    expected = _mm_set_epi8(-1, 1, 2, -3, 4, 5, -6, -7, 8, 1, 1, 1, 2, 3, 4, 5);
    PolarCode::Decoding::FastSscFip::F_function(llr, &child, 16);
    CPPUNIT_ASSERT(testVectors(child, expected));
    PolarCode::Decoding::FixedDecoding::F_function<16>(llr, &child);
    CPPUNIT_ASSERT(testVectors(child, expected));

    // G-function
    llr[1] = _mm_set_epi8(-1, 2, 3, -4, 5, 6, -7, 8, 9, 0, 1, 2, 3, 4, 5, 6);
    llr[0] = _mm_set_epi8(0, 1, 2, 3, 4, 5, 6, -7, 8, 9, 0, 1, 2, 3, 4, 5);
    bits = _mm_set_epi8(0, 0, 0, -128, 0, 0, -128, -128, 0, 0, 0, 0, 0, 0, 0, 0);
    expected = _mm_set_epi8(-1, 3, 5, -7, 9, 11, -13, 15, 17, 9, 1, 3, 5, 7, 9, 11);
    PolarCode::Decoding::FastSscFip::G_function(llr, &child, &bits, 16);
    CPPUNIT_ASSERT(testVectors(child, expected));
    PolarCode::Decoding::FixedDecoding::G_function<16>(llr, &child, &bits);
    CPPUNIT_ASSERT(testVectors(child, expected));

    // Combine-function
    llr[0] =
        _mm_setr_epi8(127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0);
    llr[1] = _mm_setr_epi8(
        -128, -128, -128, -128, -128, -128, -128, -128, 0, 0, 0, 0, 0, 0, 0, 0);
    expected = _mm_setr_epi8(-127,
                             -127,
                             -127,
                             -127,
                             -127,
                             -127,
                             -127,
                             -127,
                             -127,
                             -127,
                             -127,
                             -127,
                             -127,
                             -127,
                             -127,
                             -127);
    PolarCode::Decoding::FastSscFip::CombineBitsShort(llr, llr + 1, &bits, 8);
    CPPUNIT_ASSERT(testBitVectors(bits, expected));
    PolarCode::Decoding::FixedDecoding::CombineBits<8>(llr, llr + 1, &bits);
    CPPUNIT_ASSERT(testBitVectors(bits, expected));

    // Extended combine-test
    {
        union charVec {
            __m128i v;
            char c[16];
        };

        charVec llrLeft;
        charVec llrRight;
        charVec result;
        for (int left = -128; left < 0; left++) {
            for (int right = -128; right < 0; right++) {
                llrLeft.c[0] = static_cast<char>(left);
                llrRight.c[0] = static_cast<char>(right);
                PolarCode::Decoding::FastSscFip::CombineBitsShort(
                    &llrLeft.v, &llrRight.v, &result.v, 1);
                CPPUNIT_ASSERT(result.c[0] >= 0);
                PolarCode::Decoding::FixedDecoding::CombineBits<1>(
                    &llrLeft.v, &llrRight.v, &result.v);
                CPPUNIT_ASSERT(result.c[0] >= 0);
            }
        }
        for (int left = 0; left < 128; left++) {
            for (int right = 0; right < 128; right++) {
                llrLeft.c[0] = static_cast<char>(left);
                llrRight.c[0] = static_cast<char>(right);
                PolarCode::Decoding::FastSscFip::CombineBitsShort(
                    &llrLeft.v, &llrRight.v, &result.v, 1);
                CPPUNIT_ASSERT(result.c[0] >= 0);
                PolarCode::Decoding::FixedDecoding::CombineBits<1>(
                    &llrLeft.v, &llrRight.v, &result.v);
                CPPUNIT_ASSERT(result.c[0] >= 0);
            }
        }
        for (int left = -128; left < 0; left++) {
            for (int right = 0; right < 128; right++) {
                llrLeft.c[0] = static_cast<char>(left);
                llrRight.c[0] = static_cast<char>(right);
                PolarCode::Decoding::FastSscFip::CombineBitsShort(
                    &llrLeft.v, &llrRight.v, &result.v, 1);
                // std::cout << "Left: " << left << ", Right: " << right << ", Result: "
                // << (int)result.c[0] << std::endl;
                CPPUNIT_ASSERT(result.c[0] < 0);
                PolarCode::Decoding::FixedDecoding::CombineBits<1>(
                    &llrLeft.v, &llrRight.v, &result.v);
                CPPUNIT_ASSERT(result.c[0] < 0);
            }
        }
        for (int left = 0; left < 128; left++) {
            for (int right = -128; right < 0; right++) {
                llrLeft.c[0] = static_cast<char>(left);
                llrRight.c[0] = static_cast<char>(right);
                PolarCode::Decoding::FastSscFip::CombineBitsShort(
                    &llrLeft.v, &llrRight.v, &result.v, 1);
                CPPUNIT_ASSERT(result.c[0] < 0);
                PolarCode::Decoding::FixedDecoding::CombineBits<1>(
                    &llrLeft.v, &llrRight.v, &result.v);
                CPPUNIT_ASSERT(result.c[0] < 0);
            }
        }
    }
}


#endif

void DecodingTest::testFipShort()
{
    using namespace std::chrono;
    high_resolution_clock::time_point TimeStart, TimeEnd;
    float TimeUsed;

    size_t blockLength = 8;
    std::vector<unsigned> frozenBits({ 0, 1, 2, 4 });

    float signal[] = { -5, -6, -4, 1, -4, -5, -7, 2 };
    unsigned char output = 0;

    PolarCode::Decoding::Decoder* decoder =
        new PolarCode::Decoding::FastSscFipChar(blockLength, frozenBits);

    TimeStart = high_resolution_clock::now();
    decoder->setSignal(signal);
    decoder->decode();
    TimeEnd = high_resolution_clock::now();
    decoder->getDecodedInformationBits(&output);
    CPPUNIT_ASSERT((output & 0xF0) == 0xF0);

    TimeUsed = duration_cast<duration<float>>(TimeEnd - TimeStart).count();

    std::cout << "Decoder speed for 8-bit block: " << siFormat(blockLength / TimeUsed)
              << "bps (" << siFormat(TimeUsed) << "s per block)" << std::endl;

    delete decoder;
}


void DecodingTest::testPerformance()
{
    using namespace std::chrono;
    high_resolution_clock::time_point TimeDecode, TimeInject, TimeEnd;
    float TimeUsed, TimeDecoder;
    std::vector<unsigned> frozenBits;

    const size_t blockLength = 1 << 10;

    /* Get a set of frozen bits */ {
        PolarCode::Construction::Constructor* constructor =
            new PolarCode::Construction::Bhattacharrya(blockLength, blockLength / 2);
        frozenBits = constructor->construct();
        delete constructor;
    }

    float signal[blockLength];
    // unsigned char output[blockLength/16];

    /* Generate random signal, regardless if it is a valid codeword or not */ {
        std::mt19937_64 generator;
        std::normal_distribution<float> dist(0, 20);
        for (unsigned i = 0; i < blockLength; ++i) {
            signal[i] = dist(generator);
        }
    }

    PolarCode::Decoding::Decoder* decoder =
        new PolarCode::Decoding::FastSscFipChar(blockLength, frozenBits);

    TimeInject = high_resolution_clock::now();
    decoder->setSignal(signal);
    TimeDecode = high_resolution_clock::now();
    decoder->decode();
    TimeEnd = high_resolution_clock::now();

    // decoder->getDecodedInformationBits(&output);
    // CPPUNIT_ASSERT(output == 0xF0);

    TimeUsed = duration_cast<duration<float>>(TimeEnd - TimeInject).count();
    TimeDecoder = duration_cast<duration<float>>(TimeEnd - TimeDecode).count();

    std::cout << "Decoder speed for " << blockLength
              << "-bit block: " << siFormat(blockLength / TimeUsed) << "bps ("
              << siFormat(TimeUsed) << "s per block)"
              << " [Decoder without bit injection: "
              << siFormat(blockLength / TimeDecoder) << "bps, " << siFormat(TimeDecoder)
              << "s/block]" << std::endl;

    delete decoder;

    decoder = new PolarCode::Decoding::SclFipChar(blockLength, 4, frozenBits);

    TimeInject = high_resolution_clock::now();
    decoder->setSignal(signal);
    TimeDecode = high_resolution_clock::now();
    decoder->decode();
    TimeEnd = high_resolution_clock::now();

    // decoder->getDecodedInformationBits(&output);
    // CPPUNIT_ASSERT(output == 0xF0);

    TimeUsed = duration_cast<duration<float>>(TimeEnd - TimeInject).count();
    TimeDecoder = duration_cast<duration<float>>(TimeEnd - TimeDecode).count();

    std::cout << "List decoder speed for " << blockLength
              << "-bit block: " << siFormat(blockLength / TimeUsed) << "bps ("
              << siFormat(TimeUsed) << "s per block)"
              << " [Decoder without bit injection: "
              << siFormat(blockLength / TimeDecoder) << "bps, " << siFormat(TimeDecoder)
              << "s/block]" << std::endl;

    delete decoder;
}

void DecodingTest::testListDecoder()
{
    using namespace std::chrono;
    high_resolution_clock::time_point TimeStart, TimeEnd;
    float TimeUsed;

    size_t blockLength = 8;
    size_t listSizeLimit = 4;
    std::vector<unsigned> frozenBits({ 0, 1, 2, 4 });

    float signal[] = { -5, -6, -4, 1, -4, -5, -7, 2 };
    unsigned char output = 0;

    PolarCode::Decoding::Decoder* decoder =
        new PolarCode::Decoding::SclFipChar(blockLength, listSizeLimit, frozenBits);

    TimeStart = high_resolution_clock::now();
    decoder->setSignal(signal);
    decoder->decode();
    TimeEnd = high_resolution_clock::now();
    decoder->getDecodedInformationBits(&output);
    CPPUNIT_ASSERT((output & 0xF0) == 0xF0);

    TimeUsed = duration_cast<duration<float>>(TimeEnd - TimeStart).count();

    std::cout << "Char list decoder speed for 8-bit block: "
              << siFormat(blockLength / TimeUsed) << "bps (" << siFormat(TimeUsed)
              << "s per block)" << std::endl;

    delete decoder;


    decoder =
        new PolarCode::Decoding::SclAvxFloat(blockLength, listSizeLimit, frozenBits);

    TimeStart = high_resolution_clock::now();
    decoder->setSignal(signal);
    decoder->decode();
    TimeEnd = high_resolution_clock::now();
    decoder->getDecodedInformationBits(&output);
    CPPUNIT_ASSERT((output & 0xF0) == 0xF0);

    TimeUsed = duration_cast<duration<float>>(TimeEnd - TimeStart).count();

    std::cout << "Float list decoder speed for 8-bit block: "
              << siFormat(blockLength / TimeUsed) << "bps (" << siFormat(TimeUsed)
              << "s per block)" << std::endl;

    delete decoder;
}


constexpr std::array<int, 8> frozenEight = { 1, 1, 1, 0, 1, 0, 0, 0 };
std::vector<unsigned> frozenEightIdx = { 3, 5, 6, 7 };

void DecodingTest::testTemplatized()
{
    /*
     * This is an attempt to make things compile for old GCC compilers.
     * The templated code is known to work with GCC 6 and above.
     * For older compilers, this test is redundant.
     */
    // #if defined(__GNUC__) && __GNUC__ >= 6
    PolarCode::Decoding::Decoder* decoder =
        new PolarCode::Decoding::TemplatizedFloat<8, frozenEight>(frozenEightIdx);
    // #else
    // std::vector<unsigned> frozenBits({0,1,2,4});
    // PolarCode::Decoding::Decoder *decoder =
    // 		new PolarCode::Decoding::FastSscAvxFloat(8, frozenBits);
    // #endif
    float signal[] = { -5, -6, -4, 1, -4, -5, -7, 2 };


    decoder->setSignal(signal);
    decoder->decode();
    float output[8];
    decoder->getSoftCodeword(output);

    std::cout << std::endl << "Templatized decoder output: " << std::endl << "[";
    for (int i = 0; i < 8; ++i) {
        std::cout << output[i];
        if (i != 7)
            std::cout << ", ";
    }
    std::cout << "]" << std::endl;

    delete decoder;
}

void DecodingTest::showScanTestOutput(unsigned iterationLimit, float output[8])
{
    std::cout << "Systematic SCAN decoder output for I=" << iterationLimit << ": "
              << std::endl
              << "[";
    for (int i = 0; i < 8; ++i) {
        std::cout << output[i];
        if (i != 7)
            std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

void DecodingTest::testScan()
{
    PolarCode::Decoding::Decoder* decoder =
        new PolarCode::Decoding::Scan(8, 1, { 0, 1, 2, 4 });
    float signal[] = { -5, -6, -4, 1, -4, -5, -7, 2 };
    float output[8];

    std::cout << std::endl;

    for (unsigned i = 1; i <= 2; i <<= 1) {
        dynamic_cast<PolarCode::Decoding::Scan*>(decoder)->setIterationLimit(i);
        decoder->setSignal(signal);

        decoder->setSystematic(true);
        decoder->decode();
        decoder->getSoftCodeword(output);
        showScanTestOutput(i, output);

        decoder->setSystematic(false);
        decoder->decode();
        decoder->getSoftCodeword(output);
        std::cout << "Non-";
        showScanTestOutput(i, output);
        std::cout << std::endl;
    }

    delete decoder;

    std::cout << "Fast-SSCAN Decoder:" << std::endl;
    decoder = new PolarCode::Decoding::FastSscanFloat(8, 1, { 0, 1, 2, 4 });

    for (unsigned i = 1; i <= 2; i <<= 1) {
        decoder->setSignal(signal);

        decoder->setSystematic(true);
        decoder->decode();
        for (unsigned j = 1; j < i; ++j) {
            dynamic_cast<PolarCode::Decoding::FastSscanFloat*>(decoder)->decodeAgain();
        }
        decoder->getSoftCodeword(output);
        showScanTestOutput(i, output);
        std::cout << std::endl;
    }

    delete decoder;
}
