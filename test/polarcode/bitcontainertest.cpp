#include "bitcontainertest.h"
#include <cstring>

CPPUNIT_TEST_SUITE_REGISTRATION(BitContainerTest);

void BitContainerTest::setUp() {
	mTestData = "TestData";
	mFrozenBits.insert({0,1,2,3,4,5,6,7});//Freeze first byte

	floatContainer = new PolarCode::FloatContainer();
	floatContainer->setSize(mTestData.size()*8);

	charContainer = new PolarCode::CharContainer();
	charContainer->setSize(mTestData.size()*8);

	control = new unsigned char[mTestData.size()];
}

void BitContainerTest::tearDown() {
	delete floatContainer;
	delete charContainer;
	delete [] control;
}

void BitContainerTest::testFloatContainer() {
	floatContainer->insertPacked(mTestData.data());
	floatContainer->getPacked(control);

	CPPUNIT_ASSERT(memcmp(mTestData.data(), control, mTestData.size()) == 0);
}

void BitContainerTest::testFloatContainerWithFrozenBits() {
	floatContainer->insertPackedInformation(mTestData.data(), mFrozenBits);
	floatContainer->getPackedInformation(control, mFrozenBits);

	CPPUNIT_ASSERT(memcmp(mTestData.data(), control, mTestData.size()-1) == 0);
}

void BitContainerTest::testCharContainer() {
	charContainer->insertPacked(mTestData.data());
	charContainer->getPacked(control);

	CPPUNIT_ASSERT(memcmp(mTestData.data(), control, mTestData.size()) == 0);
}

void BitContainerTest::testCharContainerWithFrozenBits() {
	charContainer->insertPackedInformation(mTestData.data(), mFrozenBits);
	charContainer->getPackedInformation(control, mFrozenBits);

	CPPUNIT_ASSERT(memcmp(mTestData.data(), control, mTestData.size()-1) == 0);
}
