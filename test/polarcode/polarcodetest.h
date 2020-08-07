/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_TEST_POLARCODE_H
#define PC_TEST_POLARCODE_H

#include <cppunit/extensions/HelperMacros.h>
#include <polarcode/polarcode.h>
#include <string>

class PolarCodeTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(PolarCodeTest);
	CPPUNIT_TEST(testAvx2);
	CPPUNIT_TEST(testAvx2List);
	CPPUNIT_TEST(testAvxConvenience);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testAvx2();
	void testAvx2List();
	void testAvxConvenience();
};

#endif //PC_TEST_POLARCODE_H
