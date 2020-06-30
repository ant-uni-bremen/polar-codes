/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/construction/bhattacharrya.h>
#include "constructiontest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ConstructionTest);

void ConstructionTest::setUp() {

}

void ConstructionTest::tearDown() {

}

void ConstructionTest::testBhattacharrya() {
	std::vector<unsigned> output;
	std::vector<unsigned> expectedOutput({0,1,2,4});

	mConstructor = new PolarCode::Construction::Bhattacharrya(8, 4);
	output = mConstructor->construct();
	delete mConstructor;
	CPPUNIT_ASSERT(output == expectedOutput);

	output.clear();

	expectedOutput = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,32,33,34,35,36,37,38,39,40,41,42,43,44,48,49,50,52,56,64,65,66,67,68,69,70,72,73,74,76,80,81,82,96};
	mConstructor = new PolarCode::Construction::Bhattacharrya(128, 64);
	output = mConstructor->construct();
	delete mConstructor;
	CPPUNIT_ASSERT(output == expectedOutput);
}
