#ifndef PC_TEST_CONSTRUCTION_H
#define PC_TEST_CONSTRUCTION_H

#include <cppunit/extensions/HelperMacros.h>
#include <polarcode/construction/bhattacharrya.h>

class ConstructionTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(ConstructionTest);
	CPPUNIT_TEST(testBhattacharrya);
	CPPUNIT_TEST_SUITE_END();

	PolarCode::Construction::Constructor *mConstructor;

public:
	void setUp();
	void tearDown();

	void testBhattacharrya();
};

#endif //PC_TEST_CONSTRUCTION_H
