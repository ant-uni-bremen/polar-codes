#include "encodingtest.h"
#include "siformat.h"

#include <polarcode/encoding/butterfly_avx_float.h>
#include <polarcode/encoding/butterfly_avx2_char.h>
#include <polarcode/encoding/butterfly_avx2_packed.h>
#include <polarcode/encoding/recursive_avx2_packed.h>
#include <polarcode/construction/bhattacharrya.h>
#include <cstring>
#include <iostream>
#include <chrono>
#include <random>

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

		frozenBits.resize(testBits-1);
		for(unsigned i=0; i<testBits-1; ++i) {
			frozenBits[i] = i;
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

		frozenBits.resize(testBits-1);
		for(unsigned i=0; i<testBits-1; ++i) {
			frozenBits[i] = i;
		}
		memset(output, 0, testBytes);

		encoder = new PolarCode::Encoding::ButterflyAvx2Packed(testBits, frozenBits);
		encoder->setInformation(&input);
		encoder->setSystematic(false);
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

void getRandomData(void *ptr, size_t length) {
	//unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	unsigned seed = 1001;
	std::mt19937_64 engine(seed);
	length /= 8;
	uint64_t *intPtr = reinterpret_cast<uint64_t*>(ptr);
	for(unsigned int i=0; i<length; ++i) {
		intPtr[i] = engine();
	}
}

void EncodingTest::avxRecursiveTest() {
	using namespace PolarCode::Construction;
	using namespace PolarCode::Encoding;
	using namespace std::chrono;

	const size_t blockLength = 4096;
	const size_t infoLength = 2048+1024;

	Constructor *constructor = new Bhattacharrya(blockLength, infoLength);
	frozenBits = constructor->construct();

	Encoder *recursiveEncoder = new RecursiveAvx2Packed(blockLength, frozenBits);
	Encoder *butterflyEncoder = new ButterflyAvx2Packed(blockLength, frozenBits);

	unsigned char *input = new unsigned char[infoLength/8]();
	unsigned char *recursiveOutput = new unsigned char[blockLength/8]();
	unsigned char *butterflyOutput = new unsigned char[blockLength/8]();

	getRandomData(input, infoLength/8);

	recursiveEncoder->setInformation(input);
	butterflyEncoder->setInformation(input);

	high_resolution_clock::time_point start, mid, end;

			start = high_resolution_clock::now();
	recursiveEncoder->encode();
			mid = high_resolution_clock::now();
	butterflyEncoder->encode();
			end = high_resolution_clock::now();

	recursiveEncoder->getEncodedData(recursiveOutput);
	butterflyEncoder->getEncodedData(butterflyOutput);

	CPPUNIT_ASSERT(0 == memcmp(recursiveOutput, butterflyOutput, blockLength/8));

	float recursiveSpeed = blockLength / duration_cast<duration<float>>(mid-start).count();
	float butterflySpeed = blockLength / duration_cast<duration<float>>(end-mid).count();

	std::cout << std::endl << "Comparison between recursive and butterfly encoders:" << std::endl
		<< "Block size: " << blockLength << " bits, information size: " << infoLength << " bits" << std::endl
		<< "Recursive encoding speed: " << siFormat(recursiveSpeed) << "bps" << std::endl
		<< "Butterfly encoding speed: " << siFormat(butterflySpeed) << "bps" << std::endl;


	delete constructor;
	delete recursiveEncoder;
	delete butterflyEncoder;
	delete [] input;
	delete [] recursiveOutput;
	delete [] butterflyOutput;
}

void EncodingTest::performanceComparison() {
	using namespace std::chrono;
	high_resolution_clock::time_point TimeStart, TimeEnd;
	duration<float> TimeUsed;

	const size_t testBits = 1<<12;

	unsigned char input = 0x80;

	frozenBits.resize(testBits-1);
	for(unsigned i=0; i<testBits-1; ++i) {
		frozenBits[i] = i;
	}

	std::cout << std::endl << "Encoding blocks of " << testBits << " bits:" << std::endl;

	encoder = new PolarCode::Encoding::ButterflyAvxFloat(testBits, frozenBits);
	encoder->setSystematic(false);
	TimeStart = high_resolution_clock::now();
	encoder->setInformation(&input);
	encoder->encode();
	TimeEnd = high_resolution_clock::now();
	delete encoder;
	TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart);
	float speed = (testBits/TimeUsed.count());
	std::cout << "AVX-Float Speed: " << siFormat(speed) << "bps (" << siFormat(speed/8) << "B/s)" << std::endl;

	if(avx2supported()) {
		encoder = new PolarCode::Encoding::ButterflyAvx2Char(testBits, frozenBits);
		encoder->setSystematic(false);
		TimeStart = high_resolution_clock::now();
		encoder->setInformation(&input);
		encoder->encode();
		TimeEnd = high_resolution_clock::now();
		delete encoder;
		TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart);
		speed = (testBits/TimeUsed.count());
		std::cout << "AVX2-Char Speed: " << siFormat(speed) << "bps (" << siFormat(speed/8) << "B/s)" << std::endl;

		encoder = new PolarCode::Encoding::ButterflyAvx2Packed(testBits, frozenBits);
		encoder->setSystematic(false);
		TimeStart = high_resolution_clock::now();
		encoder->setInformation(&input);
		encoder->encode();
		TimeEnd = high_resolution_clock::now();
		delete encoder;
		TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart);
		speed = (testBits/TimeUsed.count());
		std::cout << "AVX2-Packed Speed: " << siFormat(speed) << "bps (" << siFormat(speed/8) << "B/s)" << std::endl;
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
