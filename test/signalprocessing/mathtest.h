#ifndef PC_TEST_MATH_H
#define PC_TEST_MATH_H

#include <cppunit/extensions/HelperMacros.h>

class MathTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(MathTest);
	CPPUNIT_TEST(test);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();
	void test();
};

#endif //PC_TEST_MATH_H


