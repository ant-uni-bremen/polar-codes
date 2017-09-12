#include "decodingtest.h"
#include "siformat.h"

#include <polarcode/decoding/fastssc_avx2_char.h>
#include <polarcode/decoding/fastssc_avx_float.h>
#include <polarcode/decoding/scl_avx2_char.h>
#include <polarcode/construction/bhattacharrya.h>
#include <chrono>
#include <random>

CPPUNIT_TEST_SUITE_REGISTRATION(DecodingTest);

void DecodingTest::setUp() {

}

void DecodingTest::tearDown() {

}

bool testShortVectors(const __m256i &one, const __m256i &two, const size_t length) {
	return 0==memcmp(&one, &two, length);
}

bool testVectors(const __m256i &one, const __m256i &two) {
	return testShortVectors(one, two, 32);
}

void DecodingTest::testSpecialDecoders() {
	__m256i llr, bits, expectedResult;

	// Rate-0
	bits = _mm256_set1_epi8(-1);
	expectedResult = _mm256_setzero_si256();
	PolarCode::Decoding::FastSscAvx2::RateZeroDecode(&llr, &bits, 32);
	bits = PolarCode::Decoding::hardDecode(bits);
	CPPUNIT_ASSERT(testVectors(bits, expectedResult));

	// Rate-1
	llr            = _mm256_set_epi8(-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1);
	bits           = _mm256_set1_epi8(-1);
	expectedResult = _mm256_set_epi8(-128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	PolarCode::Decoding::FastSscAvx2::RateOneDecode(&llr, &bits, 32);
	bits = PolarCode::Decoding::hardDecode(bits);
	CPPUNIT_ASSERT(testVectors(bits, expectedResult));

	// Repetition
	llr            = _mm256_set_epi8(-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1);
	bits           = _mm256_set1_epi8(-1);
	expectedResult = _mm256_set_epi8(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	PolarCode::Decoding::FastSscAvx2::RepetitionDecode(&llr, &bits, 32);
	bits = PolarCode::Decoding::hardDecode(bits);
	CPPUNIT_ASSERT(testVectors(bits, expectedResult));

	// SPC
	llr            = _mm256_set_epi8(-1,1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1);
	bits           = _mm256_set1_epi8(-1);
	expectedResult = _mm256_set_epi8(-128,0,-128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-128,-128);
	PolarCode::Decoding::FastSscAvx2::SpcDecode(&llr, &bits, 32);
	bits = PolarCode::Decoding::hardDecode(bits);
	CPPUNIT_ASSERT(testVectors(bits, expectedResult));

	llr            = _mm256_set_epi8(-1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	bits           = _mm256_set1_epi8(-1);
	expectedResult = _mm256_set_epi8(0,-128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	PolarCode::Decoding::FastSscAvx2::SpcDecode(&llr, &bits, 2);
	CPPUNIT_ASSERT(testShortVectors(bits, expectedResult, 2));

	llr            = _mm256_set_epi8(1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	bits           = _mm256_set1_epi8(-1);
	expectedResult = _mm256_set_epi8(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	PolarCode::Decoding::FastSscAvx2::SpcDecode(&llr, &bits, 2);
	CPPUNIT_ASSERT(testShortVectors(bits, expectedResult, 2));

	/*
	std::vector<unsigned> frozenBits;
	frozenBits.resize(1,0);
	PolarCode::DataPool<__m256i, 32> pool;
	PolarCode::Decoding::FastSscAvx2::Node *base =
			new PolarCode::Decoding::FastSscAvx2::Node(2,&pool);
	PolarCode::Decoding::FastSscAvx2::RateRNode *node
			= new PolarCode::Decoding::FastSscAvx2::ShortRateRNode(frozenBits, base);
	__m256i* input = base->input();char*ci=reinterpret_cast<char*>(input);
	__m256i* output = base->output();
	__m256i spcoutput;
	int a=1;int b=-1;
//	for(int a=-127;a<128;++a) {
//		for(int b=-127;b<128;++b) {
			*input = _mm256_setzero_si256();
			ci[0] = a;
			ci[1] = b;
			node->decode(input, output);
			PolarCode::Decoding::FastSscAvx2::SpcDecode(input, &spcoutput, 2);
			if(0!=memcmp(output, &spcoutput, 2)) {
				std::cout << "Problem here." << std::endl;
			}
//		}
//	}
*/
}

void DecodingTest::testGeneralDecodingFunctions() {
	__m256i llr[2], bits, child;
	__m256i expected;

	// F-function
	llr[0]   = _mm256_set_epi8( 0, 1, 2, 3, 4, 5, 6,-7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1);
	llr[1]   = _mm256_set_epi8(-1, 2, 3,-4, 5, 6,-7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2);
	expected = _mm256_set_epi8( 0, 1, 2,-3, 4, 5,-6,-7, 8, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 1);
	PolarCode::Decoding::F_function(llr, &child, 32);
	CPPUNIT_ASSERT(testVectors(child, expected));

	// G-function
	llr[1]   = _mm256_set_epi8(-1, 2, 3,   -4, 5, 6,   -7,    8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2);
	llr[0]   = _mm256_set_epi8( 0, 1, 2,    3, 4, 5,    6,   -7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1);
	bits     = _mm256_set_epi8( 0, 0, 0, -128, 0, 0, -128, -128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	expected = _mm256_set_epi8(-1, 3, 5,   -7, 9,11,  -13,   15,17, 9, 1, 3, 5, 7, 9,11,13,15,17, 9, 1, 3, 5, 7, 9,11,13,15,17, 9, 1, 3);
	PolarCode::Decoding::G_function(llr, &child, &bits, 32);
	CPPUNIT_ASSERT(testVectors(child, expected));

	// Combine-function
	llr[0]   = _mm256_setr_epi8( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	llr[1]   = _mm256_setr_epi8( 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	expected = _mm256_setr_epi8( 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	PolarCode::Decoding::CombineShortBits(llr, llr+1, &bits, 8);
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
	CPPUNIT_ASSERT((output&0xF0) == 0xF0);

	TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart).count();

	std::cout << "Decoder speed for 8-bit block: " << siFormat(blockLength/TimeUsed) << "bps (" << siFormat(TimeUsed) << "s per block)" << std::endl;

	delete decoder;
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
		std::normal_distribution<float> dist(0, 20);
		for(unsigned i=0; i<blockLength; ++i) {
			signal[i] = dist(generator);
		}
	}

	PolarCode::Decoding::Decoder *decoder = new PolarCode::Decoding::FastSscAvxFloat(blockLength, frozenBits);

	TimeInject = high_resolution_clock::now();
	decoder->setSignal(signal);
	TimeDecode = high_resolution_clock::now();
	decoder->decode();
	TimeEnd = high_resolution_clock::now();

	//decoder->getDecodedInformationBits(&output);
	//CPPUNIT_ASSERT(output == 0xF0);

	TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeInject).count();
	TimeDecoder = duration_cast<duration<float>>(TimeEnd-TimeDecode).count();

	std::cout << "Decoder speed for " << blockLength << "-bit block: " << siFormat(blockLength/TimeUsed) << "bps (" << siFormat(TimeUsed) << "s per block)"
			  << " [Decoder without bit injection: " << siFormat(blockLength/TimeDecoder) << "bps, " << siFormat(TimeDecoder) << "s/block]" << std::endl;

	delete decoder;

	decoder = new PolarCode::Decoding::SclAvx2Char(blockLength, 4, frozenBits);

	TimeInject = high_resolution_clock::now();
	decoder->setSignal(signal);
	TimeDecode = high_resolution_clock::now();
	decoder->decode();
	TimeEnd = high_resolution_clock::now();

	//decoder->getDecodedInformationBits(&output);
	//CPPUNIT_ASSERT(output == 0xF0);

	TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeInject).count();
	TimeDecoder = duration_cast<duration<float>>(TimeEnd-TimeDecode).count();

	std::cout << "List decoder speed for " << blockLength << "-bit block: " << siFormat(blockLength/TimeUsed) << "bps (" << siFormat(TimeUsed) << "s per block)"
			  << " [Decoder without bit injection: " << siFormat(blockLength/TimeDecoder) << "bps, " << siFormat(TimeDecoder) << "s/block]" << std::endl;

	delete decoder;
}

void DecodingTest::testListDecoder() {
	using namespace std::chrono;
	high_resolution_clock::time_point TimeStart, TimeEnd;
	float TimeUsed;

	size_t blockLength = 8;
	size_t listSizeLimit = 4;
	std::vector<unsigned> frozenBits({0,1,2,4});

	float signal[]={-5, -6, -4, 1, -4, -5, -7, 2};
	unsigned char output = 0;

	PolarCode::Decoding::Decoder *decoder = new PolarCode::Decoding::SclAvx2Char(blockLength, listSizeLimit, frozenBits);

	TimeStart = high_resolution_clock::now();
	decoder->setSignal(signal);
	decoder->decode();
	TimeEnd = high_resolution_clock::now();
	decoder->getDecodedInformationBits(&output);
	CPPUNIT_ASSERT((output&0xF0) == 0xF0);

	TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart).count();

	std::cout << "List decoder speed for 8-bit block: " << siFormat(blockLength/TimeUsed) << "bps (" << siFormat(TimeUsed) << "s per block)" << std::endl;

	delete decoder;
}
