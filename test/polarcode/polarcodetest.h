#ifndef PC_TEST_POLARCODE_H
#define PC_TEST_POLARCODE_H

#include <cppunit/extensions/HelperMacros.h>
#include <polarcode/polarcode.h>
#include <string>

class PolarCodeTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(PolarCodeTest);
	CPPUNIT_TEST(testAvx2);
	CPPUNIT_TEST(testAvx2List);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testAvx2();
	void testAvx2List();
};

#endif //PC_TEST_POLARCODE_H
