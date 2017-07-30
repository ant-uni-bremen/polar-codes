#include "encodingtest.h"

#include <polarcode/encoding/butterfly_avx_float.h>
#include <polarcode/encoding/butterfly_avx2_char.h>
#include <polarcode/encoding/butterfly_avx2_packed.h>
#include <cstring>
#include <iostream>
#include <chrono>

CPPUNIT_TEST_SUITE_REGISTRATION(EncodingTest);

void EncodingTest::setUp() {

}

void EncodingTest::tearDown() {

}

void EncodingTest::avxFloatTest() {
	unsigned char input = 0x80;//One information bit at MSB
	unsigned char output = 0x00;//Set to something else than expected
	unsigned char expectedOutput = 0xFF;//Output of repetition code

	encoder = new PolarCode::Encoding::ButterflyAvxFloat(8, {0,1,2,3,4,5,6});
	encoder->setSystematic(false);
	encoder->setInformation(&input);
	encoder->encode();
	encoder->getEncodedData(&output);
	delete encoder;

	CPPUNIT_ASSERT_EQUAL(output, expectedOutput);
}

void EncodingTest::avxCharTest() {
	const size_t maxBits = 1024;
	const size_t maxBytes = maxBits/8;

	unsigned char input = 0x80;
	unsigned char output[maxBytes];
	unsigned char expectedOutput[maxBytes];

	memset(expectedOutput, 0xFF, maxBytes);

	//At first, check if AVX2 is supported
	if(!avx2supported()) {
		std::cerr << std::endl
				  << "PolarCode::Encoding::ButterflyAvx2Char can't be testet." << std::endl
				  << "AVX2 is not supported on this system." << std::endl;
		return;
	}

	for(size_t testBytes = 1; testBytes<=maxBytes; testBytes<<=1) {
		size_t testBits = testBytes*8;
//		std::cout << "Test length: " << testBytes << " Bytes, " << testBits << " Bits" << std::endl;

		frozenBits.clear();
		for(unsigned i=0; i<testBits-1; ++i) {
			frozenBits.insert(i);
		}
		memset(output, 0, testBytes);

		encoder = new PolarCode::Encoding::ButterflyAvx2Char(testBits, frozenBits);
		encoder->setInformation(&input);
		encoder->encode();
		encoder->getEncodedData(output);
		delete encoder;

		CPPUNIT_ASSERT_EQUAL(0, memcmp(output, expectedOutput, testBytes));
	}
}

void EncodingTest::avxPackedTest() {
	const size_t maxBits = 1024;
	const size_t maxBytes = maxBits/8;

	unsigned char input = 0x80;
	unsigned char output[maxBytes];
	unsigned char expectedOutput[maxBytes];

	memset(expectedOutput, 0xFF, maxBytes);

	//At first, check if AVX2 is supported
	if(!avx2supported()) {
		std::cerr << std::endl
				  << "PolarCode::Encoding::ButterflyAvx2Packed can't be testet." << std::endl
				  << "AVX2 is not supported on this system." << std::endl;
		return;
	}

	for(size_t testBytes = 1; testBytes<=maxBytes; testBytes<<=1) {
		size_t testBits = testBytes*8;
//		std::cout << "Test length: " << testBytes << " Bytes, " << testBits << " Bits" << std::endl;

		frozenBits.clear();
		for(unsigned i=0; i<testBits-1; ++i) {
			frozenBits.insert(i);
		}
		memset(output, 0, testBytes);

		encoder = new PolarCode::Encoding::ButterflyAvx2Packed(testBits, frozenBits);
		encoder->setInformation(&input);
		encoder->encode();
		encoder->getEncodedData(output);
		delete encoder;

		bool testPassed = (0 == memcmp(output, expectedOutput, testBytes));
		if(!testPassed) {
			std::cout << "Packed encoding failed for a block of size " << testBits << std::endl;
		}
		CPPUNIT_ASSERT(testPassed);
	}
}

void EncodingTest::performanceComparison() {
	using namespace std::chrono;
	high_resolution_clock::time_point TimeStart, TimeEnd;
	duration<float> TimeUsed;

	const size_t testBits = 1<<12;

	unsigned char input = 0x80;

	frozenBits.clear();
	for(unsigned i=0; i<testBits-1; ++i) {
		frozenBits.insert(i);
	}

	std::cout << "Encoding blocks of " << testBits << " bits:" << std::endl;

	encoder = new PolarCode::Encoding::ButterflyAvxFloat(testBits, frozenBits);
	encoder->setSystematic(false);
	TimeStart = high_resolution_clock::now();
	encoder->setInformation(&input);
	encoder->encode();
	TimeEnd = high_resolution_clock::now();
	delete encoder;
	TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart);
	float speed = (testBits/TimeUsed.count()/1000000);
	std::cout << "AVX-Float Speed: " <<  speed << " Mbps (" << (speed/8) << " MB/s)" << std::endl;

	if(avx2supported()) {
		encoder = new PolarCode::Encoding::ButterflyAvx2Char(testBits, frozenBits);
		encoder->setSystematic(false);
		TimeStart = high_resolution_clock::now();
		encoder->setInformation(&input);
		encoder->encode();
		TimeEnd = high_resolution_clock::now();
		delete encoder;
		TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart);
		speed = (testBits/TimeUsed.count()/1000000);
		std::cout << "AVX2-Char Speed: " <<  speed << " Mbps (" << (speed/8) << " MB/s)" << std::endl;

		encoder = new PolarCode::Encoding::ButterflyAvx2Packed(testBits, frozenBits);
		encoder->setSystematic(false);
		TimeStart = high_resolution_clock::now();
		encoder->setInformation(&input);
		encoder->encode();
		TimeEnd = high_resolution_clock::now();
		delete encoder;
		TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart);
		speed = (testBits/TimeUsed.count()/1000000);
		std::cout << "AVX2-Packed Speed: " <<  speed << " Mbps (" << (speed/8) << " MB/s)" << std::endl;
	} else {
		std::cout << "AVX2 can't be tested on this system, sorry." << std::endl;
	}
}

bool EncodingTest::avx2supported() {
	try {
		encoder = new PolarCode::Encoding::ButterflyAvx2Char();
		delete encoder;
		return true;
	} catch (Avx2NotSupportedException) {
		return false;
	}
}
