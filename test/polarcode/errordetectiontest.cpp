#include "errordetectiontest.h"

#include <cstring>

CPPUNIT_TEST_SUITE_REGISTRATION(ErrorDetectionTest);

void ErrorDetectionTest::setUp() {
	using namespace PolarCode::ErrorDetection;

	mDummy = new Dummy();
	mCrc8  = new CRC8();
	mCrc32 = new CRC32();

	mDataLength = 32;
	mData = new char[mDataLength];
	mTestInput = new unsigned char[mDataLength];
	strcpy(reinterpret_cast<char*>(mData), "Hello, this is some test text..");
}

void ErrorDetectionTest::tearDown() {
	delete mDummy;
	delete mCrc8;
	delete mCrc32;
	delete [] mData;
	delete [] mTestInput;
}

void ErrorDetectionTest::testDummy() {
	strcpy(reinterpret_cast<char*>(mTestInput), mData);

	mDummy->generate(mTestInput, mDataLength);
	CPPUNIT_ASSERT_EQUAL(true, mDummy->check(mTestInput, mDataLength));

	mTestInput[0] ^= 0xFF;
	CPPUNIT_ASSERT_EQUAL(true, mDummy->check(mTestInput, mDataLength));
}

void ErrorDetectionTest::testCrc8() {
	strcpy(reinterpret_cast<char*>(mTestInput), mData);

	mCrc8->generate(mTestInput, mDataLength);
	CPPUNIT_ASSERT_EQUAL(true, mCrc8->check(mTestInput, mDataLength));

	mTestInput[0] ^= 0xFF;
	CPPUNIT_ASSERT_EQUAL(false, mCrc8->check(mTestInput, mDataLength));
}

void ErrorDetectionTest::testCrc32() {
	strcpy(reinterpret_cast<char*>(mTestInput), mData);

	mCrc32->generate(mTestInput, mDataLength);
	CPPUNIT_ASSERT_EQUAL(true, mCrc32->check(mTestInput, mDataLength));

	mTestInput[0] ^= 0xFF;
	CPPUNIT_ASSERT_EQUAL(false, mCrc32->check(mTestInput, mDataLength));
}
