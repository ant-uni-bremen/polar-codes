#include "decodingtest.h"
#include "siformat.h"

#include <polarcode/decoding/fastssc_fip_char.h>
#include <polarcode/decoding/fastssc_avx_float.h>
#include <polarcode/decoding/scl_fip_char.h>
#include <polarcode/decoding/scl_avx_float.h>
#include <polarcode/decoding/fip_templates.txx>
#include <polarcode/decoding/templatized_float.h>
#include <polarcode/construction/bhattacharrya.h>
#include <chrono>
#include <random>

CPPUNIT_TEST_SUITE_REGISTRATION(DecodingTest);

void DecodingTest::setUp() {
}

void DecodingTest::tearDown() {

}

bool testShortVectors(const fipv &one, const fipv &two, const size_t length) {
	return 0 == memcmp(&one, &two, length);
}

bool testVectors(const fipv &one, const fipv &two) {
	return testShortVectors(one, two, BYTESPERVECTOR);
}

bool testShortVectors(const __m256 &one, const __m256 &two, const size_t length) {
	return 0 == memcmp(&one, &two, length * 4);
}

bool testVectors(const __m256 &one, const __m256 &two) {
	return testShortVectors(one, two, 8);
}

void DecodingTest::testSpecialDecoders() {
/*	__m256i llr, bits, expectedResult;

	// Rate-0
	bits = _mm256_set1_epi8(-1);
	expectedResult = _mm256_setzero_si256();
	PolarCode::Decoding::FastSscAvx2::RateZeroDecode(&llr, &bits, 32);
	bits = PolarCode::Decoding::FastSscAvx2::hardDecode(bits);
	CPPUNIT_ASSERT(testVectors(bits, expectedResult));

	// Rate-1
	llr            = _mm256_set_epi8(-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1);
	bits           = _mm256_set1_epi8(-1);
	expectedResult = _mm256_set_epi8(-128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	PolarCode::Decoding::FastSscAvx2::RateOneDecode(&llr, &bits, 32);
	bits = PolarCode::Decoding::FastSscAvx2::hardDecode(bits);
	CPPUNIT_ASSERT(testVectors(bits, expectedResult));

	// Repetition
	llr            = _mm256_set_epi8(-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1);
	bits           = _mm256_set1_epi8(-1);
	expectedResult = _mm256_set_epi8(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	PolarCode::Decoding::FastSscAvx2::RepetitionDecode(&llr, &bits, 32);
	bits = PolarCode::Decoding::FastSscAvx2::hardDecode(bits);
	CPPUNIT_ASSERT(testVectors(bits, expectedResult));

	// SPC
	llr            = _mm256_set_epi8(-1,1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1);
	bits           = _mm256_set1_epi8(-1);
	expectedResult = _mm256_set_epi8(-128,0,-128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-128,-128);
	PolarCode::Decoding::FastSscAvx2::SpcDecode(&llr, &bits, 32);
	bits = PolarCode::Decoding::FastSscAvx2::hardDecode(bits);
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
*/}

void DecodingTest::testGeneralDecodingFunctionsAvx() {
	union fv {
		__m256 v;
		float f[8];
	};

	fv llr[2], bits, child;
	fv expected;

	// F-function
	llr[0].v   = _mm256_set_ps( 0, 1, 2, 3, 4, 5, 6,-7);
	llr[1].v   = _mm256_set_ps( 1, 2, 3,-4, 5, 6,-7, 8);
	expected.v = _mm256_set_ps( 0, 1, 2,-3, 4, 5,-6,-7);
	PolarCode::Decoding::FastSscAvx::F_function(&llr[0].f[0], &child.f[0], 8);
	CPPUNIT_ASSERT(testVectors(child.v, expected.v));

	// G-function
	llr[1].v   = _mm256_set_ps(-1, 2, 3,   -4, 5, 6,   -7,    8);
	llr[0].v   = _mm256_set_ps( 0, 1, 2,    3, 4, 5,    6,   -7);
	bits.v     = _mm256_set_ps( 0, 0, 0, -128, 0, 0, -128, -128);
	expected.v = _mm256_set_ps(-1, 3, 5,   -7, 9,11,  -13,   15);
	PolarCode::Decoding::FastSscAvx::G_function(&llr[0].f[0], &child.f[0], &bits.f[0], 8);
	CPPUNIT_ASSERT(testVectors(child.v, expected.v));

	// Combine-function
	llr[0].v   = _mm256_setr_ps(  1,  1,  1,  1,  0,  0,  0,  0);
	llr[1].v   = _mm256_setr_ps( -1, -1, -1, -1,  0,  0,  0,  0);
	expected.v = _mm256_setr_ps( -1, -1, -1, -1, -1, -1, -1, -1);
	PolarCode::Decoding::FastSscAvx::CombineSoftBitsShort(&llr[0].f[0], &llr[1].f[0], &bits.f[0], 4);
	CPPUNIT_ASSERT(testVectors(bits.v, expected.v));
}

#ifdef __AVX2__

void DecodingTest::testGeneralDecodingFunctionsAvx2() {
	__m256i llr[2], bits, child;
	__m256i expected;

	// F-function
	llr[0]   = _mm256_set_epi8( 0, 1, 2, 3, 4, 5, 6,-7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1);
	llr[1]   = _mm256_set_epi8(-1, 2, 3,-4, 5, 6,-7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2);
	expected = _mm256_set_epi8(-1, 1, 2,-3, 4, 5,-6,-7, 8, 1, 1, 1, 2, 3, 4, 5, 6, 7, 8, 1, 1, 1, 2, 3, 4, 5, 6, 7, 8, 1, 1, 1);
	PolarCode::Decoding::FastSscFip::F_function(llr, &child, 32);
	CPPUNIT_ASSERT(testVectors(child, expected));
	PolarCode::Decoding::FixedDecoding::F_function<32>(llr, &child);
	CPPUNIT_ASSERT(testVectors(child, expected));

	// G-function
	llr[1]   = _mm256_set_epi8(-1, 2, 3,   -4, 5, 6,   -7,    8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2);
	llr[0]   = _mm256_set_epi8( 0, 1, 2,    3, 4, 5,    6,   -7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1);
	bits     = _mm256_set_epi8( 0, 0, 0, -128, 0, 0, -128, -128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	expected = _mm256_set_epi8(-1, 3, 5,   -7, 9,11,  -13,   15,17, 9, 1, 3, 5, 7, 9,11,13,15,17, 9, 1, 3, 5, 7, 9,11,13,15,17, 9, 1, 3);
	PolarCode::Decoding::FastSscFip::G_function(llr, &child, &bits, 32);
	CPPUNIT_ASSERT(testVectors(child, expected));
	PolarCode::Decoding::FixedDecoding::G_function<32>(llr, &child, &bits);
	CPPUNIT_ASSERT(testVectors(child, expected));

	// Combine-function
	llr[0]   = _mm256_setr_epi8(   127,   127,   127,   127,   127,   127,   127,   127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	llr[1]   = _mm256_setr_epi8(  -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	expected = _mm256_setr_epi8(  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
	PolarCode::Decoding::FastSscFip::CombineSoftBitsShort(llr, llr+1, &bits, 8);
	CPPUNIT_ASSERT(testVectors(bits, expected));
	PolarCode::Decoding::FixedDecoding::CombineSoftBits<8>(llr, llr+1, &bits);
	CPPUNIT_ASSERT(testVectors(bits, expected));

	//Extended combine-test
	{
		union charVec {
			__m256i v;
			char c[32];
		};

		charVec llrLeft;
		charVec llrRight;
		charVec result;
		for(int left = -128; left < 0; left++) {
			for(int right = -128; right < 0; right++) {
				llrLeft.c[0] = static_cast<char>(left);
				llrRight.c[0] = static_cast<char>(right);
				PolarCode::Decoding::FastSscFip::CombineSoftBitsShort(&llrLeft.v, &llrRight.v, &result.v, 1);
				CPPUNIT_ASSERT(result.c[0] >= 0);
				PolarCode::Decoding::FixedDecoding::CombineSoftBits<1>(&llrLeft.v, &llrRight.v, &result.v);
				CPPUNIT_ASSERT(result.c[0] >= 0);
			}
		}
		for(int left = 0; left < 128; left++) {
			for(int right = 0; right < 128; right++) {
				llrLeft.c[0] = static_cast<char>(left);
				llrRight.c[0] = static_cast<char>(right);
				PolarCode::Decoding::FastSscFip::CombineSoftBitsShort(&llrLeft.v, &llrRight.v, &result.v, 1);
				CPPUNIT_ASSERT(result.c[0] >= 0);
				PolarCode::Decoding::FixedDecoding::CombineSoftBits<1>(&llrLeft.v, &llrRight.v, &result.v);
				CPPUNIT_ASSERT(result.c[0] >= 0);
			}
		}
		for(int left = -128; left < 0; left++) {
			for(int right = 0; right < 128; right++) {
				llrLeft.c[0] = static_cast<char>(left);
				llrRight.c[0] = static_cast<char>(right);
				PolarCode::Decoding::FastSscFip::CombineSoftBitsShort(&llrLeft.v, &llrRight.v, &result.v, 1);
				//std::cout << "Left: " << left << ", Right: " << right << ", Result: " << (int)result.c[0] << std::endl;
				CPPUNIT_ASSERT(result.c[0] < 0);
				PolarCode::Decoding::FixedDecoding::CombineSoftBits<1>(&llrLeft.v, &llrRight.v, &result.v);
				CPPUNIT_ASSERT(result.c[0] < 0);
			}
		}
		for(int left = 0; left < 128; left++) {
			for(int right = -128; right < 0; right++) {
				llrLeft.c[0] = static_cast<char>(left);
				llrRight.c[0] = static_cast<char>(right);
				PolarCode::Decoding::FastSscFip::CombineSoftBitsShort(&llrLeft.v, &llrRight.v, &result.v, 1);
				CPPUNIT_ASSERT(result.c[0] < 0);
				PolarCode::Decoding::FixedDecoding::CombineSoftBits<1>(&llrLeft.v, &llrRight.v, &result.v);
				CPPUNIT_ASSERT(result.c[0] < 0);
			}
		}
	}
}

#else

void DecodingTest::testGeneralDecodingFunctionsSse() {
	__m128i llr[2], bits, child;
	__m128i expected;

	// F-function
	llr[0]   = _mm_set_epi8( 0, 1, 2, 3, 4, 5, 6,-7, 8, 9, 0, 1, 2, 3, 4, 5);
	llr[1]   = _mm_set_epi8(-1, 2, 3,-4, 5, 6,-7, 8, 9, 0, 1, 2, 3, 4, 5, 6);
	expected = _mm_set_epi8(-1, 1, 2,-3, 4, 5,-6,-7, 8, 1, 1, 1, 2, 3, 4, 5);
	PolarCode::Decoding::FastSscFip::F_function(llr, &child, 16);
	CPPUNIT_ASSERT(testVectors(child, expected));
	PolarCode::Decoding::FixedDecoding::F_function<16>(llr, &child);
	CPPUNIT_ASSERT(testVectors(child, expected));

	// G-function
	llr[1]   = _mm_set_epi8(-1, 2, 3,   -4, 5, 6,   -7,    8, 9, 0, 1, 2, 3, 4, 5, 6);
	llr[0]   = _mm_set_epi8( 0, 1, 2,    3, 4, 5,    6,   -7, 8, 9, 0, 1, 2, 3, 4, 5);
	bits     = _mm_set_epi8( 0, 0, 0, -128, 0, 0, -128, -128, 0, 0, 0, 0, 0, 0, 0, 0);
	expected = _mm_set_epi8(-1, 3, 5,   -7, 9,11,  -13,   15,17, 9, 1, 3, 5, 7, 9,11);
	PolarCode::Decoding::FastSscFip::G_function(llr, &child, &bits, 16);
	CPPUNIT_ASSERT(testVectors(child, expected));
	PolarCode::Decoding::FixedDecoding::G_function<16>(llr, &child, &bits);
	CPPUNIT_ASSERT(testVectors(child, expected));

	// Combine-function
	llr[0]   = _mm_setr_epi8(   127,   127,   127,   127,   127,   127,   127,   127, 0, 0, 0, 0, 0, 0, 0, 0);
	llr[1]   = _mm_setr_epi8(  -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128, 0, 0, 0, 0, 0, 0, 0, 0);
	expected = _mm_setr_epi8(  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127);
	PolarCode::Decoding::FastSscFip::CombineSoftBitsShort(llr, llr + 1, &bits, 8);
	CPPUNIT_ASSERT(testVectors(bits, expected));
	PolarCode::Decoding::FixedDecoding::CombineSoftBits<8>(llr, llr + 1, &bits);
	CPPUNIT_ASSERT(testVectors(bits, expected));

	//Extended combine-test
	{
		union charVec {
			__m128i v;
			char c[16];
		};

		charVec llrLeft;
		charVec llrRight;
		charVec result;
		for(int left = -128; left < 0; left++) {
			for(int right = -128; right < 0; right++) {
				llrLeft.c[0] = static_cast<char>(left);
				llrRight.c[0] = static_cast<char>(right);
				PolarCode::Decoding::FastSscFip::CombineSoftBitsShort(&llrLeft.v, &llrRight.v, &result.v, 1);
				CPPUNIT_ASSERT(result.c[0] >= 0);
				PolarCode::Decoding::FixedDecoding::CombineSoftBits<1>(&llrLeft.v, &llrRight.v, &result.v);
				CPPUNIT_ASSERT(result.c[0] >= 0);
			}
		}
		for(int left = 0; left < 128; left++) {
			for(int right = 0; right < 128; right++) {
				llrLeft.c[0] = static_cast<char>(left);
				llrRight.c[0] = static_cast<char>(right);
				PolarCode::Decoding::FastSscFip::CombineSoftBitsShort(&llrLeft.v, &llrRight.v, &result.v, 1);
				CPPUNIT_ASSERT(result.c[0] >= 0);
				PolarCode::Decoding::FixedDecoding::CombineSoftBits<1>(&llrLeft.v, &llrRight.v, &result.v);
				CPPUNIT_ASSERT(result.c[0] >= 0);
			}
		}
		for(int left = -128; left < 0; left++) {
			for(int right = 0; right < 128; right++) {
				llrLeft.c[0] = static_cast<char>(left);
				llrRight.c[0] = static_cast<char>(right);
				PolarCode::Decoding::FastSscFip::CombineSoftBitsShort(&llrLeft.v, &llrRight.v, &result.v, 1);
				//std::cout << "Left: " << left << ", Right: " << right << ", Result: " << (int)result.c[0] << std::endl;
				CPPUNIT_ASSERT(result.c[0] < 0);
				PolarCode::Decoding::FixedDecoding::CombineSoftBits<1>(&llrLeft.v, &llrRight.v, &result.v);
				CPPUNIT_ASSERT(result.c[0] < 0);
			}
		}
		for(int left = 0; left < 128; left++) {
			for(int right = -128; right < 0; right++) {
				llrLeft.c[0] = static_cast<char>(left);
				llrRight.c[0] = static_cast<char>(right);
				PolarCode::Decoding::FastSscFip::CombineSoftBitsShort(&llrLeft.v, &llrRight.v, &result.v, 1);
				CPPUNIT_ASSERT(result.c[0] < 0);
				PolarCode::Decoding::FixedDecoding::CombineSoftBits<1>(&llrLeft.v, &llrRight.v, &result.v);
				CPPUNIT_ASSERT(result.c[0] < 0);
			}
		}
	}
}


#endif

void DecodingTest::testFipShort() {
	using namespace std::chrono;
	high_resolution_clock::time_point TimeStart, TimeEnd;
	float TimeUsed;

	size_t blockLength = 8;
	std::vector<unsigned> frozenBits({0,1,2,4});

	float signal[]={-5, -6, -4, 1, -4, -5, -7, 2};
	unsigned char output = 0;

	PolarCode::Decoding::Decoder *decoder = new PolarCode::Decoding::FastSscFipChar(blockLength, frozenBits);

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

void DecodingTest::testPerformance() {
	using namespace std::chrono;
	high_resolution_clock::time_point TimeDecode, TimeInject, TimeEnd;
	float TimeUsed, TimeDecoder;
	std::vector<unsigned> frozenBits;

	const size_t blockLength = 1<<10;

	/* Get a set of frozen bits */ {
		PolarCode::Construction::Constructor *constructor
				= new PolarCode::Construction::Bhattacharrya(blockLength, blockLength/2);
		frozenBits = constructor->construct();
		delete constructor;
	}

	float signal[blockLength];
	//unsigned char output[blockLength/16];

	/* Generate random signal, regardless if it is a valid codeword or not */ {
		std::mt19937_64 generator;
		std::normal_distribution<float> dist(0, 20);
		for(unsigned i=0; i<blockLength; ++i) {
			signal[i] = dist(generator);
		}
	}

	PolarCode::Decoding::Decoder *decoder = new PolarCode::Decoding::FastSscFipChar(blockLength, frozenBits);

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

	decoder = new PolarCode::Decoding::SclFipChar(blockLength, 4, frozenBits);

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

	PolarCode::Decoding::Decoder *decoder = new PolarCode::Decoding::SclFipChar(blockLength, listSizeLimit, frozenBits);

	TimeStart = high_resolution_clock::now();
	decoder->setSignal(signal);
	decoder->decode();
	TimeEnd = high_resolution_clock::now();
	decoder->getDecodedInformationBits(&output);
	CPPUNIT_ASSERT((output&0xF0) == 0xF0);

	TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart).count();

	std::cout << "Char list decoder speed for 8-bit block: " << siFormat(blockLength/TimeUsed) << "bps (" << siFormat(TimeUsed) << "s per block)" << std::endl;

	delete decoder;


	decoder = new PolarCode::Decoding::SclAvxFloat(blockLength, listSizeLimit, frozenBits);

	TimeStart = high_resolution_clock::now();
	decoder->setSignal(signal);
	decoder->decode();
	TimeEnd = high_resolution_clock::now();
	decoder->getDecodedInformationBits(&output);
	CPPUNIT_ASSERT((output&0xF0) == 0xF0);

	TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart).count();

	std::cout << "Float list decoder speed for 8-bit block: " << siFormat(blockLength/TimeUsed) << "bps (" << siFormat(TimeUsed) << "s per block)" << std::endl;

	delete decoder;

}


constexpr std::array<int, 8> frozenEight = {1, 1, 1, 0, 1, 0, 0, 0};

void DecodingTest::testTemplatized() {
	PolarCode::Decoding::Decoder *decoder =
	        new PolarCode::Decoding::TemplatizedFloat<8, frozenEight>();
	float signal[]={-5, -6, -4, 1, -4, -5, -7, 2};


	decoder->setSignal(signal);
	decoder->decode();
	float output[8];
	decoder->getSoftCodeword(output);

	std::cout << std::endl << "Templatized decoder output: " << std::endl << "[";
	for(int i = 0; i < 8; ++i) {
		std::cout << output[i];
		if(i != 7) std::cout << ", ";
	}
	std::cout << "]" << std::endl;

	delete decoder;
}
