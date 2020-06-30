/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_TEST_BITCONTAINER_H
#define PC_TEST_BITCONTAINER_H

#include <cppunit/extensions/HelperMacros.h>
#include <polarcode/bitcontainer.h>

class BitContainerTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(BitContainerTest);
	CPPUNIT_TEST(testFloatContainer);
	CPPUNIT_TEST(testFloatContainerWithFrozenBits);
	CPPUNIT_TEST(testCharContainer);
	CPPUNIT_TEST(testCharContainerWithFrozenBits);
	CPPUNIT_TEST_SUITE_END();

	PolarCode::BitContainer  *floatContainer;
	PolarCode::BitContainer   *charContainer;
	PolarCode::BitContainer *packedContainer;
	unsigned char *control;
	std::string mTestData;
	std::vector<unsigned> mFrozenBits;

public:
	void setUp();
	void tearDown();

	void testFloatContainer();
	void testFloatContainerWithFrozenBits();
	void testCharContainer();
	void testCharContainerWithFrozenBits();
	void testPackedContainer();
	void testPackedContainerWithFrozenBits();
};

#endif //PC_TEST_BITCONTAINER_H
