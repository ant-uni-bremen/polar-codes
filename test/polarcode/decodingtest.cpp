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
	std::vector<unsigned> frozenBits({0,1,2,4});

	float signal[]={-5, -6, -4, 1, -4, -5, -7, 2};
	unsigned char output = 0;

	PolarCode::Decoding::Decoder *decoder = new PolarCode::Decoding::FastSscAvx2Char(blockLength, frozenBits);

	TimeStart = high_resolution_clock::now();
	decoder->setSignal(signal);
	decoder->decode();
	TimeEnd = high_resolution_clock::now();
	decoder->getDecodedInformationBits(&output);
	CPPUNIT_ASSERT(output == 0xF0);

	TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart).count();

	std::cout << "Decoder speed for 8-bit block: " << (blockLength/1e6/TimeUsed) << " Mbps (" << (TimeUsed*1e9) << " ns per block)" << std::endl;

	delete decoder;

}

void DecodingTest::testAvx2Performance() {
	using namespace std::chrono;
	high_resolution_clock::time_point TimeDecode, TimeInject, TimeEnd;
	float TimeUsed, TimeDecoder;
	std::vector<unsigned> frozenBits;

	const size_t blockLength = 1<<12;

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

	TimeInject = high_resolution_clock::now();
	decoder->setSignal(signal);
	TimeDecode = high_resolution_clock::now();
	decoder->decode();
	TimeEnd = high_resolution_clock::now();

	//decoder->getDecodedInformationBits(&output);
	//CPPUNIT_ASSERT(output == 0xF0);

	TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeInject).count();
	TimeDecoder = duration_cast<duration<float>>(TimeEnd-TimeDecode).count();

	std::cout << "Decoder speed for " << blockLength << "-bit block: " << (blockLength*1e-6/TimeUsed) << " Mbps (" << (TimeUsed*1e9) << " ns per block)"
			  << " [Decoder without bit injection: " << (blockLength*1e-6/TimeDecoder) << " Mbps, " << (TimeDecoder*1e9) << " ns/block]" << std::endl;

	delete decoder;

}
