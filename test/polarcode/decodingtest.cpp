#include "decodingtest.h"

#include <polarcode/decoding/fastssc_avx2_char.h>
#include <polarcode/encoding/butterfly_avx2_packed.h>
#include <polarcode/construction/bhattacharrya.h>
#include <chrono>
#include <iostream>
#include <random>

CPPUNIT_TEST_SUITE_REGISTRATION(DecodingTest);

void DecodingTest::setUp() {

}

void DecodingTest::tearDown() {

}

bool testVectors(const __m256i &one, const __m256i &two) {
	return 0==memcmp(&one, &two, 32);
}

void DecodingTest::testSpecialDecoders() {
	__m256i llr, bits, expectedResult;

	// Rate-0
	bits = _mm256_set1_epi8(-1);
	expectedResult = _mm256_setzero_si256();
	PolarCode::Decoding::FastSscAvx2::RateZeroDecode(&llr, &bits, 32);
	CPPUNIT_ASSERT(testVectors(bits, expectedResult));

	// Rate-1
	llr            = _mm256_set_epi8(-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1);
	bits           = _mm256_set1_epi8(-1);
	expectedResult = _mm256_set_epi8(1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	PolarCode::Decoding::FastSscAvx2::RateOneDecode(&llr, &bits, 32);
	CPPUNIT_ASSERT(testVectors(bits, expectedResult));

	// Repetition
	llr            = _mm256_set_epi8(-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1);
	bits           = _mm256_set1_epi8(-1);
	expectedResult = _mm256_set_epi8(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	PolarCode::Decoding::FastSscAvx2::RepetitionDecode(&llr, &bits, 32);
	CPPUNIT_ASSERT(testVectors(bits, expectedResult));

	// SPC
	llr            = _mm256_set_epi8(-1,1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1);
	bits           = _mm256_set1_epi8(-1);
	expectedResult = _mm256_set_epi8(1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1);
	PolarCode::Decoding::FastSscAvx2::SpcDecode(&llr, &bits, 32);
	CPPUNIT_ASSERT(testVectors(bits, expectedResult));
}

void DecodingTest::testGeneralDecodingFunctions() {
	__m256i llr[2], bits, child;
	__m256i expected;

	// F-function
	llr[0]   = _mm256_set_epi8( 0, 1, 2, 3, 4, 5, 6,-7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1);
	llr[1]   = _mm256_set_epi8(-1, 2, 3,-4, 5, 6,-7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2);
	expected = _mm256_set_epi8( 0, 1, 2,-3, 4, 5,-6,-7, 8, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 1);
	PolarCode::Decoding::FastSscAvx2::F_function(llr, &child, 32);
	CPPUNIT_ASSERT(testVectors(child, expected));

	// G-function
	llr[1]   = _mm256_set_epi8(-1, 2, 3,-4, 5, 6, -7,  8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2);
	llr[0]   = _mm256_set_epi8( 0, 1, 2, 3, 4, 5,  6, -7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1);
	bits     = _mm256_set_epi8( 0, 0, 0, 1, 0, 0,  1,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	expected = _mm256_set_epi8(-1, 3, 5,-7, 9,11,-13, 15,17, 9, 1, 3, 5, 7, 9,11,13,15,17, 9, 1, 3, 5, 7, 9,11,13,15,17, 9, 1, 3);
	PolarCode::Decoding::FastSscAvx2::G_function(llr, &child, &bits, 32);
	CPPUNIT_ASSERT(testVectors(child, expected));

	// Combine-function
	llr[0]   = _mm256_setr_epi8( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	llr[1]   = _mm256_setr_epi8( 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	expected = _mm256_setr_epi8( 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	PolarCode::Decoding::FastSscAvx2::CombineShortBits(llr, llr+1, &bits, 8);
	CPPUNIT_ASSERT(testVectors(bits, expected));
}

void DecodingTest::testAvx2Short() {
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

void bpskModulate(unsigned char *input, char *output, size_t blockLength) {
	unsigned char currentByte = 0;// Initialize to suppress false warning
	for(unsigned i=0; i<blockLength; ++i) {
		if(i%8==0) {
			currentByte = *(input++);
		}
		if(currentByte&0x80) {
			*output = -1;
		} else {
			*output = 1;
		}
		currentByte <<= 1;
		output++;
	}
}

void DecodingTest::testAvx2() {
	const size_t blockLength = 1<<12;
	const size_t infoLength = blockLength*3/4;

	unsigned char *input = new unsigned char[infoLength/8];
	unsigned char *inputBlock = new unsigned char[blockLength/8];
	char *inputSignal = new char[blockLength];
	unsigned char *output = new unsigned char[infoLength/8];

	PolarCode::Construction::Constructor* constructor
			= new PolarCode::Construction::Bhattacharrya(blockLength, infoLength);
	std::vector<unsigned> frozenBits = constructor->construct();

	PolarCode::Encoding::ButterflyAvx2Packed* encoder
			= new PolarCode::Encoding::ButterflyAvx2Packed(blockLength, frozenBits);

	PolarCode::Decoding::FastSscAvx2Char* decoder
			= new PolarCode::Decoding::FastSscAvx2Char(blockLength, frozenBits);

	memset(input, 0xF0, infoLength/8);

	encoder->setInformation(input);
	encoder->encode();
	encoder->getEncodedData(inputBlock);

	{// Test if encoded bits are still recoverable from systematic codeword
		PolarCode::PackedContainer *cont = new PolarCode::PackedContainer(blockLength, frozenBits);
		cont->insertPackedBits(inputBlock);
		cont->getPackedInformationBits(output);
		delete cont;
		CPPUNIT_ASSERT(0 == memcmp(input, output, infoLength/8));
	}

	bpskModulate(inputBlock, inputSignal, blockLength);

	decoder->setSignal(inputSignal);
	CPPUNIT_ASSERT(decoder->decode());
	decoder->getDecodedInformationBits(output);

	CPPUNIT_ASSERT(0 == memcmp(input, output, infoLength/8));



	delete encoder;
	delete constructor;
	delete [] input;
	delete [] inputSignal;
	delete [] output;
}

void DecodingTest::testAvx2Performance() {
	using namespace std::chrono;
	high_resolution_clock::time_point TimeDecode, TimeInject, TimeEnd;
	float TimeUsed, TimeDecoder;
	std::vector<unsigned> frozenBits;

	const size_t blockLength = 1<<16;

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
