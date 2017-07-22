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

	PolarCode::BitContainer *floatContainer;
	PolarCode::BitContainer  *charContainer;
	unsigned char *control;
	std::string mTestData;
	std::set<unsigned> mFrozenBits;

public:
	void setUp();
	void tearDown();

	void testFloatContainer();
	void testFloatContainerWithFrozenBits();
	void testCharContainer();
	void testCharContainerWithFrozenBits();
};

#endif //PC_TEST_BITCONTAINER_H
