#include "decodingtest.h"

#include <polarcode/decoding/fastssc_avx2_char.h>

CPPUNIT_TEST_SUITE_REGISTRATION(DecodingTest);

void DecodingTest::setUp() {

}

void DecodingTest::tearDown() {

}

void DecodingTest::testAvx2Repetition() {
	mDecoder = new PolarCode::Decoding::FastSscAvx2Char();
}
