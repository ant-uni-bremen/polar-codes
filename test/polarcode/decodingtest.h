#ifndef PC_TEST_DECODING_H
#define PC_TEST_DECODING_H

#include <cppunit/extensions/HelperMacros.h>
#include <polarcode/decoding/decoder.h>

class DecodingTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(DecodingTest);
	CPPUNIT_TEST(testSpecialDecoders);
	CPPUNIT_TEST(testGeneralDecodingFunctionsAvx);
	CPPUNIT_TEST(testGeneralDecodingFunctionsAvx2);
	CPPUNIT_TEST(testAvx2Short);
	CPPUNIT_TEST(testAvx2Performance);
	CPPUNIT_TEST(testListDecoder);
	CPPUNIT_TEST_SUITE_END();

	PolarCode::Decoding::Decoder *mDecoder;

public:
	void setUp();
	void tearDown();

	void testSpecialDecoders();
	void testGeneralDecodingFunctionsAvx();
	void testGeneralDecodingFunctionsAvx2();
	void testAvx2Short();
	void testAvx2Performance();

	void testListDecoder();
};

#endif //PC_TEST_DECODING_H
