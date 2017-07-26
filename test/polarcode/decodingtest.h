#ifndef PC_TEST_DECODING_H
#define PC_TEST_DECODING_H

#include <cppunit/extensions/HelperMacros.h>
#include <polarcode/decoding/decoder.h>

class DecodingTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(DecodingTest);
	CPPUNIT_TEST(testAvx2Repetition);
	CPPUNIT_TEST_SUITE_END();

	PolarCode::Decoding::Decoder *mDecoder;

public:
	void setUp();
	void tearDown();

	void testAvx2Repetition();
};

#endif //PC_TEST_DECODING_H
