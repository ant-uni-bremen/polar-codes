/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_TEST_ENCODING_H
#define PC_TEST_ENCODING_H

#include <cppunit/extensions/HelperMacros.h>
#include <polarcode/encoding/encoder.h>

class EncodingTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(EncodingTest);
	CPPUNIT_TEST(fipPackedTest);
	CPPUNIT_TEST(fipPackedTestShort);
	CPPUNIT_TEST(fipRecursiveTest);
	CPPUNIT_TEST(performanceComparison);
	CPPUNIT_TEST_SUITE_END();

	PolarCode::Encoding::Encoder *encoder;
	std::vector<unsigned> frozenBits;

public:
	void setUp();
	void tearDown();

	void fipPackedTest();
	void fipPackedTestShort();
	void fipRecursiveTest();
	void performanceComparison();
};

#endif //PC_TEST_ENCODING_H
