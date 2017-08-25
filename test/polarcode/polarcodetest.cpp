#include "polarcodetest.h"

#include <polarcode/construction/bhattacharrya.h>
#include <polarcode/errordetection/crc32.h>
#include <polarcode/encoding/butterfly_avx2_packed.h>
#include <polarcode/decoding/fastssc_avx2_char.h>
#include <polarcode/decoding/scl_avx2_char.h>

#include <random>
#include <iostream>
#include <iomanip>

CPPUNIT_TEST_SUITE_REGISTRATION(PolarCodeTest);

void PolarCodeTest::setUp() {

}

void PolarCodeTest::tearDown() {

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


void PolarCodeTest::testAvx2() {
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

	//memset(input, 0xF0, infoLength/8);
	{
		std::mt19937 generator;
		uint32_t* inputPtr = reinterpret_cast<uint32_t*>(input);
		for(unsigned i=0; i<infoLength/32; ++i) {
			inputPtr[i] = generator();
		}
	}

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

void PolarCodeTest::testAvx2List() {
	const size_t blockLength = 1<<12;
	const size_t infoLength = blockLength*3/4;
	const size_t pathLimit = 4;

	unsigned char *input = new unsigned char[infoLength/8];
	unsigned char *inputBlock = new unsigned char[blockLength/8];
	char *inputSignal = new char[blockLength];
	unsigned char *output = new unsigned char[infoLength/8];

	PolarCode::Construction::Constructor* constructor
			= new PolarCode::Construction::Bhattacharrya(blockLength, infoLength);
	std::vector<unsigned> frozenBits = constructor->construct();

	PolarCode::Encoding::Encoder* encoder
			= new PolarCode::Encoding::ButterflyAvx2Packed(blockLength, frozenBits);

	PolarCode::ErrorDetection::Detector* errorDetector
			= new PolarCode::ErrorDetection::CRC32();

	PolarCode::Decoding::Decoder* decoder
			= new PolarCode::Decoding::SclAvx2Char(blockLength, pathLimit, frozenBits);
	decoder->setErrorDetection(errorDetector);

	{
		//Generate random data
		std::mt19937 generator;
		uint32_t* inputPtr = reinterpret_cast<uint32_t*>(input);
		for(unsigned i=0; i<infoLength/32; ++i) {
			inputPtr[i] = generator();
		}

		//Apply outer code for error detection
		errorDetector->generate(input, infoLength/8);
	}


	encoder->setInformation(input);
	encoder->encode();
	encoder->getEncodedData(inputBlock);

	{// Test if encoded bits are still recoverable from systematic codeword
		PolarCode::BitContainer *cont = new PolarCode::PackedContainer(blockLength, frozenBits);
		cont->insertPackedBits(inputBlock);
		cont->getPackedInformationBits(output);
		delete cont;
		CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE("Encoder or BitContainer failed",
		CPPUNIT_ASSERT(0 == memcmp(input, output, infoLength/8)));
	}

	bpskModulate(inputBlock, inputSignal, blockLength);

	decoder->setSignal(inputSignal);
	/*CPPUNIT_ASSERT(*/decoder->decode()/*)*/;
	decoder->getDecodedInformationBits(output);

/*	{
		uint8_t* inputPtr = reinterpret_cast<uint8_t*>(input);
		uint8_t* outputPtr = reinterpret_cast<uint8_t*>(output);
		std::cout << std::endl << "[";
		for(unsigned i=0; i<infoLength/8; ++i) {
			std::cout << std::setw(3) << (unsigned)inputPtr[i] << std::setw(0) << " ";
		}
		std::cout << "\b]" << std::endl << "[";
		for(unsigned i=0; i<infoLength/8; ++i) {
			std::cout << std::setw(3) << (unsigned)outputPtr[i] << std::setw(0) << " ";
		}
		std::cout << "\b]" << std::endl;
	}*/

	bool decoderSuccess = (0==memcmp(input, output, infoLength/8));

	delete encoder;
	delete constructor;
	delete [] input;
	delete [] inputSignal;
	delete [] output;

	CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE("Decoder failed",
	CPPUNIT_ASSERT(decoderSuccess));
}
