#include "decodingtest.h"

#include <polarcode/decoding/fastssc_avx2_char.h>
#include <polarcode/construction/bhattacharrya.h>
#include <chrono>
#include <iostream>
#include <random>

CPPUNIT_TEST_SUITE_REGISTRATION(DecodingTest);

void DecodingTest::setUp() {

}

void DecodingTest::tearDown() {

}

void DecodingTest::testAvx2() {
	using namespace std::chrono;
	high_resolution_clock::time_point TimeStart, TimeEnd;
	float TimeUsed;

	size_t blockLength = 8;
	std::set<unsigned> frozenBits({0,1,2,4});

	float signal[]={-5, -6, -4, 1, -4, -5, -7, 2};
	unsigned char output = 0;

	PolarCode::Decoding::Decoder *decoder = new PolarCode::Decoding::FastSscAvx2Char(blockLength, frozenBits);

	decoder->setSignal(signal);
	decoder->decode();
	decoder->setSignal(signal);

	TimeStart = high_resolution_clock::now();
	decoder->decode();
	TimeEnd = high_resolution_clock::now();
	decoder->getDecodedInformationBits(&output);
	CPPUNIT_ASSERT(output == 0xF0);

	TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart).count();

	std::cout << "Decoder speed for 8-bit block: " << (8.0/TimeUsed/1000000.0) << " Mbps (" << (TimeUsed*1000000000) << " ns per block)" << std::endl;

	delete decoder;

}

void DecodingTest::testAvx2Performance() {
	using namespace std::chrono;
	high_resolution_clock::time_point TimeStart, TimeEnd;
	duration<float> TimeUsed;
	std::set<unsigned> frozenBits;

	const size_t blockLength = 1<<10;

	/* Get a set of frozen bits */ {
		PolarCode::Construction::Constructor *constructor
				= new PolarCode::Construction::Bhattacharrya(blockLength, blockLength/2);
		frozenBits = constructor->construct();
		delete constructor;
	}

	float signal[blockLength];
	//unsigned char output[blockLength/2];

	/* Generate random signal, regardless if it is a valid codeword or not */ {
		std::mt19937_64 generator;
		std::normal_distribution<float> dist(0, 10);
		for(unsigned i=0; i<blockLength; ++i) {
			signal[i] = dist(generator);
		}
	}

	PolarCode::Decoding::Decoder *decoder = new PolarCode::Decoding::FastSscAvx2Char(blockLength, frozenBits);

	decoder->setSignal(signal);
	TimeStart = high_resolution_clock::now();
	decoder->decode();
	TimeEnd = high_resolution_clock::now();
	//decoder->getDecodedInformationBits(&output);
	//CPPUNIT_ASSERT(output == 0xF0);

	TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart);

	std::cout << "Decoder speed for " << blockLength << "-bit block: " << (8.0/TimeUsed.count()/1000000.0) << " Mbps (" << TimeUsed.count()*1000000000 << " ns per block)" << std::endl;

	delete decoder;

}
