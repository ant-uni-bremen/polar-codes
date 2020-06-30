/* -*- c++ -*- */
/*
 * Copyright 2020 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "puncturertest.h"
#include <cstring>
#include <numeric>
#include <algorithm>
#include <bitset>
// #include <sstream>

CPPUNIT_TEST_SUITE_REGISTRATION(PuncturerTest);

void PuncturerTest::setUp() {

	mFrozenBits = {0, 1, 2, 3, 4, 5, 6, 7,
				   9, 10, 11, 12, 13, 14,
				   16, 17, 18, 19, 20, 21, 22,
				   24, 25, 26, 27, 28, 29, 30,};//Freeze first byte
	puncturer = new PolarCode::Puncturer(230, mFrozenBits);

}

void PuncturerTest::tearDown() {
	delete puncturer;

}

void PuncturerTest::testRoundUpPowerOf2() {
	CPPUNIT_ASSERT(PolarCode::round_up_power_of_two(5) == 8);
	CPPUNIT_ASSERT(PolarCode::round_up_power_of_two(8) == 8);
	CPPUNIT_ASSERT(PolarCode::round_up_power_of_two(11) == 16);
	CPPUNIT_ASSERT(PolarCode::round_up_power_of_two(16) == 16);
	CPPUNIT_ASSERT(PolarCode::round_up_power_of_two(230) == 256);
	CPPUNIT_ASSERT(PolarCode::round_up_power_of_two(128) == 128);
	CPPUNIT_ASSERT(PolarCode::round_up_power_of_two(511) == 512);
	CPPUNIT_ASSERT(PolarCode::round_up_power_of_two(1500) == 2048);
	CPPUNIT_ASSERT(PolarCode::round_up_power_of_two(4096) == 4096);
	CPPUNIT_ASSERT(puncturer->parentBlockLength() == 256);

	// for(int i = 3; i < 69; ++i){
	// 	std::cout << "round_up_power_of_two("<< i << ") = " << PolarCode::round_up_power_of_two(i) << std::endl;
	// }
}

void PuncturerTest::testInverseSetDifference(){
	std::vector<unsigned> v1 {1, 2, 5, 7};
    std::vector<unsigned> v2 {0, 3, 4, 6};

    auto res = PolarCode::inverse_set_difference(8, v1);
    CPPUNIT_ASSERT(res == v2);
    // std::cout << "the result:\n";
    // for (auto i : res) std::cout << i << ' ';
    // std::cout << std::endl;

	std::vector<unsigned> testSet = std::vector<unsigned>({0, 1, 2, 4, 5, 7});
	std::vector<unsigned> inverseSet = std::vector<unsigned>({3, 6});

    res = PolarCode::inverse_set_difference(8, testSet);
    CPPUNIT_ASSERT(inverseSet == res);
    // std::cout << "the result:\n";
    // for (auto i : res) std::cout << i << ' ';
    // std::cout << std::endl;
}

void PuncturerTest::testOutputPositions(){
	std::vector<unsigned> frozenPos {0, 1, 2, 4};
	std::vector<unsigned> expected {2, 3, 4, 5, 6, 7};
	PolarCode::Puncturer punc(6, frozenPos);
	auto outPos = punc.blockOutputPositions();
	CPPUNIT_ASSERT(expected == outPos);

	// std::cout << "the result:\n";
 //    for (auto i : outPos) std::cout << i << ' ';
 //    std::cout << std::endl;

	std::vector<unsigned> frozenPos2 {0, 1, 4, 5, 6, 7};
	std::vector<unsigned> expected2 {2, 3, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	PolarCode::Puncturer punc2(12, frozenPos2);
	auto outPos2 = punc2.blockOutputPositions();

	// std::cout << "the result:\n";
 //    for (auto i : outPos2) std::cout << i << ' ';
 //    std::cout << std::endl;

	CPPUNIT_ASSERT(expected2 == outPos2);
}

void PuncturerTest::testPuncturing(){
	std::vector<unsigned> frozenPos {0, 1, 2, 4};
	PolarCode::Puncturer punc(6, frozenPos);

	std::vector<unsigned char> input {0, 1, 2, 3, 4, 5, 6, 7};
	std::vector<unsigned char> expected {2, 3, 4, 5, 6, 7};
	std::vector<unsigned char> result(6);
	punc.puncture(result.data(), input.data());

	CPPUNIT_ASSERT(result == expected);


	std::vector<unsigned> frozenPosLarge {0, 1, 2, 4, 5, 7, 8, 12};
	PolarCode::Puncturer puncLarge(24, frozenPosLarge);
	std::vector<unsigned char> v(32);
    std::iota(v.begin(), v.end(), 0);
    std::vector<unsigned char> resultLarge(24);
    puncLarge.puncture(resultLarge.data(), v.data());

    std::vector<unsigned char> diff;
    std::set_difference(v.begin(), v.end(),
                        frozenPosLarge.begin(), frozenPosLarge.end(),
                        std::inserter(diff, diff.begin()));
    CPPUNIT_ASSERT(resultLarge == diff);
}

void PuncturerTest::testPuncturingPacked(){
	std::vector<unsigned> frozenPosLarge {0, 1, 2, 4, 5, 7, 8, 12};
	PolarCode::Puncturer punc(24, frozenPosLarge);
	// 11110000 00001111 01010101 10101010
	std::vector<unsigned char> input {0xF0, 0x0F, 0x55, 0xAA};
	// 10000111 01010101 10101010
	std::vector<unsigned char> expected {0b10000111, 0b01010101, 0b10101010};

	std::cout << "the input:\n";
    for (auto i : input) std::cout << std::bitset<8>(i).to_string() << ' ';
    std::cout << std::endl;

    std::vector<unsigned char> result(3);
    punc.puncturePacked(result.data(), input.data());

    std::cout << "the result:\n";
    for (auto i : result) std::cout << std::bitset<8>(i).to_string() << ' ';
    std::cout << std::endl;

    CPPUNIT_ASSERT(expected == result);
}

void PuncturerTest::testDepuncturingInt(){
	std::vector<unsigned> frozenPos {0, 1, 2, 4};
	PolarCode::Puncturer punc(6, frozenPos);

	std::vector<unsigned> input {2, 3, 4, 5, 6, 7};
	std::vector<unsigned> expected {0, 0, 2, 3, 4, 5, 6, 7};

	std::vector<unsigned> result(8, 99);
	punc.depuncture<unsigned>(result.data(), input.data());

	CPPUNIT_ASSERT(result == expected);
}

void PuncturerTest::testDepuncturingFloat(){
	std::vector<unsigned> frozenPosLarge {0, 1, 2, 4, 5, 7, 8, 12};
	PolarCode::Puncturer puncLarge(24, frozenPosLarge);

	std::vector<float> v(24);
	std::iota(v.begin(), v.end(), 0);
    std::vector<float> resultLarge(32, -10.0f);

    puncLarge.depuncture(resultLarge.data(), v.data());

    // std::cout << "the result:\n";
    // for (auto i : resultLarge) std::cout << i << ' ';
    // std::cout << std::endl;

	auto outPos = puncLarge.blockOutputPositions();
	auto vi = v.begin();
	for(auto p : outPos){
		CPPUNIT_ASSERT(resultLarge[p] == *vi++);
	}

}
