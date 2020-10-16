/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "encodingtest.h"
#include "siformat.h"

#include <polarcode/construction/bhattacharrya.h>
#include <polarcode/encoding/butterfly_fip_packed.h>
#include <polarcode/encoding/recursive_fip_packed.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <random>

CPPUNIT_TEST_SUITE_REGISTRATION(EncodingTest);

void EncodingTest::setUp() {}

void EncodingTest::tearDown() {}

void EncodingTest::fipPackedTest()
{
    const size_t maxBits = 1024;
    const size_t maxBytes = maxBits / 8;

    unsigned char input = 0x80;
    unsigned char output[maxBytes];
    unsigned char expectedOutput[maxBytes];

    memset(expectedOutput, 0xFF, maxBytes);

    for (size_t testBytes = 1; testBytes <= maxBytes; testBytes <<= 1) {
        size_t testBits = testBytes * 8;
        //		std::cout << "Test length: " << testBytes << " Bytes, " << testBits
        //<< " Bits" << std::endl;

        frozenBits.resize(testBits - 1);
        for (unsigned i = 0; i < testBits - 1; ++i) {
            frozenBits[i] = i;
        }
        memset(output, 0, testBytes);

        encoder = new PolarCode::Encoding::ButterflyFipPacked(testBits, frozenBits);
        encoder->setInformation(&input);
        encoder->setSystematic(false);
        encoder->encode();
        encoder->getEncodedData(output);
        delete encoder;

        bool testPassed = (0 == memcmp(output, expectedOutput, testBytes));
        if (!testPassed) {
            std::cout << "Packed encoding failed for a block of size " << testBits
                      << std::endl;
            for (unsigned i = 0; i < testBytes; ++i) {
                std::cout << "E/O: " << (int)expectedOutput[i] << "/" << (int)output[i]
                          << std::endl;
            }
        }
        CPPUNIT_ASSERT(testPassed);
    }
}

void EncodingTest::fipPackedTestShort()
{
    {
        auto constructor = new PolarCode::Construction::Bhattacharrya(16, 8);
        frozenBits = constructor->construct();
        delete constructor;
    }
    unsigned char input = 0xFF;
    unsigned char output[2];
    unsigned char expectedOutput[2] = { 0xFF, 0xFF };

    encoder = new PolarCode::Encoding::ButterflyFipPacked(16, frozenBits);
    encoder->setSystematic(true);

    encoder->setInformation(&input);
    encoder->encode();
    encoder->getEncodedData(output);

    delete encoder;

    CPPUNIT_ASSERT(output[0] == expectedOutput[0] && output[1] == expectedOutput[1]);
}

void getRandomData(void* ptr, size_t length)
{
    // unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    unsigned seed = 1001;
    std::mt19937_64 engine(seed);
    length /= 8;
    uint64_t* intPtr = reinterpret_cast<uint64_t*>(ptr);
    for (unsigned int i = 0; i < length; ++i) {
        intPtr[i] = engine();
    }
}

void EncodingTest::fipRecursiveTest()
{
    using namespace PolarCode::Construction;
    using namespace PolarCode::Encoding;
    using namespace std::chrono;

    const size_t blockLength = 4096;
    const size_t infoLength = 2048 + 1024;

    Constructor* constructor = new Bhattacharrya(blockLength, infoLength);
    frozenBits = constructor->construct();

    Encoder* recursiveEncoder = new RecursiveFipPacked(blockLength, frozenBits);
    Encoder* butterflyEncoder = new ButterflyFipPacked(blockLength, frozenBits);

    unsigned char* input = new unsigned char[infoLength / 8]();
    unsigned char* recursiveOutput = new unsigned char[blockLength / 8]();
    unsigned char* butterflyOutput = new unsigned char[blockLength / 8]();

    getRandomData(input, infoLength / 8);

    recursiveEncoder->setInformation(input);
    butterflyEncoder->setInformation(input);

    high_resolution_clock::time_point start, mid, end;

    start = high_resolution_clock::now();
    recursiveEncoder->encode();
    mid = high_resolution_clock::now();
    butterflyEncoder->encode();
    end = high_resolution_clock::now();

    recursiveEncoder->getEncodedData(recursiveOutput);
    butterflyEncoder->getEncodedData(butterflyOutput);

    CPPUNIT_ASSERT(0 == memcmp(recursiveOutput, butterflyOutput, blockLength / 8));

    float recursiveSpeed =
        blockLength / duration_cast<duration<float>>(mid - start).count();
    float butterflySpeed =
        blockLength / duration_cast<duration<float>>(end - mid).count();

    std::cout << std::endl
              << "Comparison between recursive and butterfly encoders:" << std::endl
              << "Block size: " << blockLength
              << " bits, information size: " << infoLength << " bits" << std::endl
              << "Recursive encoding speed: " << siFormat(recursiveSpeed) << "bps"
              << std::endl
              << "Butterfly encoding speed: " << siFormat(butterflySpeed) << "bps"
              << std::endl;


    delete constructor;
    delete recursiveEncoder;
    delete butterflyEncoder;
    delete[] input;
    delete[] recursiveOutput;
    delete[] butterflyOutput;
}

void EncodingTest::performanceComparison()
{
    using namespace std::chrono;
    high_resolution_clock::time_point TimeStart, TimeEnd;
    duration<float> TimeUsed;

    const size_t testBits = 1 << 12;

    unsigned char input = 0x80;

    frozenBits.resize(testBits - 1);
    for (unsigned i = 0; i < testBits - 1; ++i) {
        frozenBits[i] = i;
    }

    std::cout << std::endl << "Encoding blocks of " << testBits << " bits:" << std::endl;


    encoder = new PolarCode::Encoding::ButterflyFipPacked(testBits, frozenBits);
    encoder->setSystematic(false);
    TimeStart = high_resolution_clock::now();
    encoder->setInformation(&input);
    encoder->encode();
    TimeEnd = high_resolution_clock::now();
    delete encoder;
    TimeUsed = duration_cast<duration<float>>(TimeEnd - TimeStart);
    float speed = (testBits / TimeUsed.count());
    std::cout << "AVX2-Packed Speed: " << siFormat(speed) << "bps ("
              << siFormat(speed / 8) << "B/s)" << std::endl;
}
