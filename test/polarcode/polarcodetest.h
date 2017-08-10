#ifndef PC_TEST_POLARCODE_H
#define PC_TEST_POLARCODE_H

#include <cppunit/extensions/HelperMacros.h>
#include <polarcode/polarcode.h>
#include <string>

class PolarCodeTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(PolarCodeTest);
//	CPPUNIT_TEST(test);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

//	void test();
};

#endif //PC_TEST_POLARCODE_H
