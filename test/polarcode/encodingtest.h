#ifndef PC_TEST_ENCODING_H
#define PC_TEST_ENCODING_H

#include <cppunit/extensions/HelperMacros.h>
#include <polarcode/encoding/encoder.h>

class EncodingTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(EncodingTest);
	CPPUNIT_TEST(avxFloatTest);
	CPPUNIT_TEST(avxCharTest);
	CPPUNIT_TEST(avxPackedTest);
	CPPUNIT_TEST(avxRecursiveTest);
	CPPUNIT_TEST(performanceComparison);
	CPPUNIT_TEST_SUITE_END();

	PolarCode::Encoding::Encoder *encoder;
	std::set<unsigned> frozenBits;

	bool avx2supported();

public:
	void setUp();
	void tearDown();

	void avxFloatTest();
	void avxCharTest();
	void avxPackedTest();
	void avxRecursiveTest();
	void performanceComparison();
};

#endif //PC_TEST_ENCODING_H
