/* -*- c++ -*- */
/*
 * Copyright 2020 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_TEST_BITCONTAINER_H
#define PC_TEST_BITCONTAINER_H

#include <cppunit/extensions/HelperMacros.h>
#include <polarcode/puncturer.h>

class PuncturerTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(PuncturerTest);
	CPPUNIT_TEST(testRoundUpPowerOf2);
	CPPUNIT_TEST(testInverseSetDifference);
	CPPUNIT_TEST(testOutputPositions);
	CPPUNIT_TEST(testPuncturing);
	CPPUNIT_TEST(testPuncturingPacked);
	CPPUNIT_TEST(testDepuncturingInt);
	CPPUNIT_TEST(testDepuncturingFloat);
	CPPUNIT_TEST_SUITE_END();

	PolarCode::Puncturer *puncturer;
	std::vector<unsigned> mFrozenBits;

public:
	void setUp();
	void tearDown();

	void testRoundUpPowerOf2();
	void testInverseSetDifference();
	void testOutputPositions();
	void testPuncturing();
	void testPuncturingPacked();
	void testDepuncturingInt();
	void testDepuncturingFloat();

};

#endif //PC_TEST_BITCONTAINER_H
