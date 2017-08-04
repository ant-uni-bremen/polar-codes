#include "bitcontainertest.h"
#include <cstring>

CPPUNIT_TEST_SUITE_REGISTRATION(BitContainerTest);

void BitContainerTest::setUp() {
	mTestData = "TestData";
	mFrozenBits = {0,1,2,3,4,5,6,7};//Freeze first byte

	floatContainer = new PolarCode::FloatContainer(mTestData.size()*8, mFrozenBits);
	 charContainer = new PolarCode::CharContainer(mTestData.size()*8, mFrozenBits);

	control = new unsigned char[mTestData.size()];
}

void BitContainerTest::tearDown() {
	delete floatContainer;
	delete charContainer;
	delete [] control;
}

void BitContainerTest::testFloatContainer() {
	floatContainer->insertPackedBits(mTestData.data());
	floatContainer->getPackedBits(control);

	CPPUNIT_ASSERT(memcmp(mTestData.data(), control, mTestData.size()) == 0);
}

void BitContainerTest::testFloatContainerWithFrozenBits() {
	floatContainer->insertPackedInformationBits(mTestData.data());
	floatContainer->getPackedInformationBits(control);

	CPPUNIT_ASSERT(memcmp(mTestData.data(), control, mTestData.size()-1) == 0);
}

void BitContainerTest::testCharContainer() {
	charContainer->insertPackedBits(mTestData.data());
	charContainer->getPackedBits(control);

	CPPUNIT_ASSERT(memcmp(mTestData.data(), control, mTestData.size()) == 0);
}

void BitContainerTest::testCharContainerWithFrozenBits() {
	charContainer->insertPackedInformationBits(mTestData.data());
	charContainer->getPackedInformationBits(control);

	CPPUNIT_ASSERT(memcmp(mTestData.data(), control, mTestData.size()-1) == 0);
}
