/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_TEST_CONSTRUCTION_H
#define PC_TEST_CONSTRUCTION_H

#include <cppunit/extensions/HelperMacros.h>
#include <polarcode/construction/constructor.h>

class ConstructionTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(ConstructionTest);
    CPPUNIT_TEST(testBhattacharrya);
    CPPUNIT_TEST_SUITE_END();

    PolarCode::Construction::Constructor* mConstructor;

public:
    void setUp();
    void tearDown();

    void testBhattacharrya();
};

#endif // PC_TEST_CONSTRUCTION_H
