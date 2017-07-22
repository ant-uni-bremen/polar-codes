#ifndef PC_TEST_ENCODING_H
#define PC_TEST_ENCODING_H

#include <cppunit/extensions/HelperMacros.h>
#include <polarcode/encoding/encoder.h>

class EncodingTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(EncodingTest);
	CPPUNIT_TEST(avxFloatTest);
	CPPUNIT_TEST(avxCharTest);
	CPPUNIT_TEST_SUITE_END();

	PolarCode::Encoding::Encoder *encoder;
	std::set<unsigned> frozenBits;

public:
	void setUp();
	void tearDown();

	void avxFloatTest();
	void avxCharTest();
};

#endif //PC_TEST_ENCODING_H
