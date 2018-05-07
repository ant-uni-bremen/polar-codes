#include <polarcode/encoding/encoder.h>
#include <polarcode/errordetection/dummy.h>
#include <chrono>
#include <iostream>

namespace PolarCode {
namespace Encoding {

Encoder::Encoder()
	: mErrorDetector(&ErrorDetection::globalDummyDetector)
	, mBlockLength(0)
	, mSystematic(true)
	, mCodewordReady(false)
	, xmInputData(nullptr)
	, mBitContainer(nullptr)
	, mFrozenBits(std::vector<unsigned>())
{
}

Encoder::~Encoder() {
	if(mBitContainer != nullptr) {
		delete mBitContainer;
	}
}

size_t Encoder::blockLength() {
	return mBlockLength;
}

void Encoder::setErrorDetection(ErrorDetection::Detector *pDetector) {
	mErrorDetector = pDetector;
}

void Encoder::setSystematic(bool sys) {
	mSystematic = sys;
}

bool Encoder::isSystematic() {
	return mSystematic;
}

void Encoder::setInformation(void *pData) {
	xmInputData = static_cast<unsigned char*>(pData);
}

void Encoder::getInformation(void *pData) {
	mBitContainer->getPackedInformationBits(pData);
}

void Encoder::setCodeword(void *pData) {
	xmInputData = static_cast<unsigned char*>(pData);
	mBitContainer->insertPackedBits(pData);
	mCodewordReady = true;
}

void Encoder::setCharCodeword(void *cData) {
	mBitContainer->insertCharBits(cData);
	mCodewordReady = true;
}

void Encoder::setFloatCodeword(void *fData) {
	mBitContainer->insertLlr((float*)fData);
	mCodewordReady = true;
}

void Encoder::getEncodedData(void *pData) {
	mBitContainer->getPackedBits(pData);
}

void Encoder::clearFrozenBits() {
	mBitContainer->resetFrozenBits();
}

void Encoder::encode_vector(void *pInfo, void *pCode){
	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
	setInformation(pInfo);
	encode();
	getEncodedData(pCode);
	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	mEncoderDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

UndefinedEncoder::UndefinedEncoder() {
}

UndefinedEncoder::~UndefinedEncoder() {
}

void UndefinedEncoder::initialize(size_t, const std::vector<unsigned> &) {
}

void UndefinedEncoder::encode() {
	std::cerr << "Call to UndefinedEncoder::encode()!" << std::endl;
}

}//namespace Encoding
}//namespace PolarCode
