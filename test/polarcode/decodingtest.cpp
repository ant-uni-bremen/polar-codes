#include "decodingtest.h"

#include <polarcode/decoding/fastssc_avx2_char.h>
#include <chrono>
#include <iostream>

CPPUNIT_TEST_SUITE_REGISTRATION(DecodingTest);

void DecodingTest::setUp() {

}

void DecodingTest::tearDown() {

}

void DecodingTest::testAvx2() {
	using namespace std::chrono;
	high_resolution_clock::time_point TimeStart, TimeEnd;
	duration<float> TimeUsed;

	size_t blockLength = 8;
	std::set<unsigned> frozenBits({0,1,2,4});

	float signal[]={-5, -6, -4, 1, -4, -5, -7, 2};
	unsigned char output = 0;

	PolarCode::Decoding::Decoder *decoder = new PolarCode::Decoding::FastSscAvx2Char(blockLength, frozenBits);

	decoder->setSignal(signal);
	TimeStart = high_resolution_clock::now();
	decoder->decode();
	TimeEnd = high_resolution_clock::now();
	decoder->getDecodedInformationBits(&output);
	CPPUNIT_ASSERT(output == 0xF0);



	std::cout << "Decoder speed for 8-bit block: " << (8.0/TimeUsed.count()/1000000.0) << " Mbps (" << TimeUsed.count()*1000000000 << " ns per block)" << std::endl;

	delete decoder;

}
