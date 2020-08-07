/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#ifndef PC_TEST_TRANSMISSION_H
#define PC_TEST_TRANSMISSION_H

#include <cppunit/extensions/HelperMacros.h>

class TransmissionTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(TransmissionTest);
//	CPPUNIT_TEST(test);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();
};

#endif //PC_TEST_TRANSMISSION_H
