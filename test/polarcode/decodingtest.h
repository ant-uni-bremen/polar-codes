#ifndef PC_TEST_DECODING_H
#define PC_TEST_DECODING_H

#include <cppunit/extensions/HelperMacros.h>
#include <polarcode/decoding/decoder.h>

class DecodingTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(DecodingTest);
	CPPUNIT_TEST(testSpecialDecoders);
	CPPUNIT_TEST(testGeneralDecodingFunctionsAvx);
#ifdef __AVX2__
	CPPUNIT_TEST(testGeneralDecodingFunctionsAvx2);
#else
	CPPUNIT_TEST(testGeneralDecodingFunctionsSse);
#endif
	CPPUNIT_TEST(testFipShort);
	CPPUNIT_TEST(testPerformance);
	CPPUNIT_TEST(testListDecoder);

	CPPUNIT_TEST(testTemplatized);

	CPPUNIT_TEST_SUITE_END();

	PolarCode::Decoding::Decoder *mDecoder;

public:
	void setUp();
	void tearDown();

	void testSpecialDecoders();
	void testGeneralDecodingFunctionsAvx();
#ifdef __AVX2__
	void testGeneralDecodingFunctionsAvx2();
#else
	void testGeneralDecodingFunctionsSse();
#endif
	void testFipShort();
	void testPerformance();

	void testListDecoder();

	void testTemplatized();
};

#endif //PC_TEST_DECODING_H
