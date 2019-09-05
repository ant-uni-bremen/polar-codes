#ifndef PC_TEST_ERRORDETECTION_H
#define PC_TEST_ERRORDETECTION_H

#include <cppunit/extensions/HelperMacros.h>

#include <polarcode/errordetection/dummy.h>
#include <polarcode/errordetection/crc8.h>
#include <polarcode/errordetection/crc32.h>

class ErrorDetectionTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(ErrorDetectionTest);
	CPPUNIT_TEST(testDummy);
	CPPUNIT_TEST(testCrc8);
	CPPUNIT_TEST(testCrc32);
	CPPUNIT_TEST_SUITE_END();

	PolarCode::ErrorDetection::Detector
		*mDummy,
		*mCrc8,
		*mCrc32;

	size_t mDataLength;
	char *mData;
	unsigned char *mTestInput;

	void generalTest(PolarCode::ErrorDetection::Detector* detector);

public:
	void setUp();
	void tearDown();

	void testDummy();
	void testCrc8();
	void testCrc32();
};

#endif //PC_TEST_ERRORDETECTION_H
