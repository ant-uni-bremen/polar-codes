/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_TEST_MODULATION_H
#define PC_TEST_MODULATION_H

#include <cppunit/extensions/HelperMacros.h>

class ModulationTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(ModulationTest);
	CPPUNIT_TEST(testBpsk);
	CPPUNIT_TEST(testAsk);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testBpsk();
	void testAsk();
};

#endif //PC_TEST_MODULATION_H
