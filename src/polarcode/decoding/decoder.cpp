#include <polarcode/decoding/decoder.h>
#include <polarcode/errordetection/dummy.h>
#include <polarcode/errordetection/crc8.h>
#include <cstring>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <polarcode/decoding/fastssc_fip_char.h>
#include <polarcode/decoding/fastssc_avx_float.h>
#include <polarcode/decoding/scl_fip_char.h>
#include <polarcode/decoding/scl_avx_float.h>
#include <polarcode/decoding/adaptive_float.h>
#include <polarcode/decoding/scan.h>

namespace PolarCode {
namespace Decoding {

Decoder* create(size_t blockLength, size_t listSize,
				const std::vector<unsigned> &frozenBits,
				std::string decoderType) {
	std::transform(decoderType.begin(), decoderType.end(), decoderType.begin(),
				   [](unsigned char c){return std::tolower(c); } );
	int decoderFlag = 0;
	if(decoderType.find("char") != std::string::npos){
		decoderFlag = 0;
	}
	else if(decoderType.find("float") != std::string::npos)
	{
		decoderFlag = 1;
	}
	else if(decoderType.find("mixed") != std::string::npos)
	{
		decoderFlag = 2;
	}
	else if(decoderType.find("scan") != std::string::npos)
	{
		decoderFlag = 3;
	}
	else{
		throw std::logic_error("Unknown PolarDecoder type!");
	}

	if(listSize < 2 && decoderFlag != 0){
		decoderFlag = 1;
	}
	return makeDecoder(blockLength, listSize, frozenBits, decoderFlag);
}

Decoder* makeDecoder(size_t blockLength, size_t listSize, const std::vector<unsigned> &frozenBits, int decoder_impl){
  Decoder* dec;
  if(listSize == 1){
	switch(decoder_impl){
	  case 1: dec = new FastSscAvxFloat(blockLength, frozenBits); break;
	  default: dec = new FastSscFipChar(blockLength, frozenBits); break;
	}
  }
  else{
	switch(decoder_impl){
	  case 1: dec = new SclAvxFloat(blockLength, listSize, frozenBits); break;
	  case 2: dec = new AdaptiveFloat(blockLength, listSize, frozenBits); break;
      case 3: dec = new Scan(blockLength, listSize, frozenBits); break;
	  default: dec = new SclFipChar(blockLength, listSize, frozenBits); break;
	}
  }
  dec->setErrorDetection(new ErrorDetection::CRC8());
  return dec;
}

Decoder::Decoder()
	: mErrorDetector(&ErrorDetection::globalDummyDetector),
	  mBlockLength(0),
	  mSystematic(true),
	  mLlrContainer(nullptr),
	  mBitContainer(nullptr),
	  mOutputContainer(nullptr),
	  mFrozenBits({}),
	  mExternalContainers(false){
}

Decoder::~Decoder() {
	if(!mExternalContainers) {
		if(mLlrContainer) delete mLlrContainer;
		if(mBitContainer) delete mBitContainer;
		if(mOutputContainer) delete [] mOutputContainer;
	}
}

void Decoder::initialize(size_t blockLength, const std::vector<unsigned> &frozenBits) {
	mBlockLength = blockLength;
	mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
}

size_t Decoder::blockLength() {
	return mBlockLength;
}

size_t Decoder::infoLength() {
	return mBlockLength - mFrozenBits.size();
}

BitContainer* Decoder::inputContainer() {
	return mLlrContainer;
}

BitContainer* Decoder::outputContainer() {
	return mBitContainer;
}

unsigned char* Decoder::packedOutput() {
	return mOutputContainer;
}

void Decoder::setSystematic(bool sys) {
	mSystematic = sys;
}

bool Decoder::isSystematic() {
	return mSystematic;
}

void Decoder::setErrorDetection(ErrorDetection::Detector *pDetector) {
	mErrorDetector = pDetector;
}

void Decoder::setSignal(const float *pLlr) {
	mLlrContainer->insertLlr(pLlr);
}

void Decoder::setSignal(const char *pLlr) {
	mLlrContainer->insertLlr(pLlr);
}

void Decoder::getDecodedInformationBits(void *pData) {
	memcpy(pData, mOutputContainer, (mBlockLength - mFrozenBits.size() + 7) / 8);
}

void Decoder::getSoftCodeword(void *pData) {
	mBitContainer->getSoftBits(pData);
}

void Decoder::getSoftInformation(void *pData) {
	mBitContainer->getSoftInformation(pData);
}

bool Decoder::decode_vector(const float *pLlr, void* pData){
//	std::cout << "float decoder CPP\n";
  std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
  setSignal(pLlr);
  bool res = decode();
  getDecodedInformationBits(pData);
  std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
  mDecoderDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  return res;
}

bool Decoder::decode_vector(const char *pLlr, void* pData) {
  std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
  setSignal(pLlr);
  bool res = decode();
  getDecodedInformationBits(pData);
  std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
  mDecoderDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  return res;
}

UndefinedDecoder::UndefinedDecoder() {
}

UndefinedDecoder::~UndefinedDecoder() {
}

bool UndefinedDecoder::decode() {
	std::cerr << "Call to UndefinedDecoder::decode()!" << std::endl;
	return false;
}

}//namespace Decoding
}//namespace PolarCode
