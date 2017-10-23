#include <polarcode/decoding/decoder.h>
#include <polarcode/errordetection/dummy.h>
#include <cstring>
#include <chrono>
#include <polarcode/decoding/fastssc_avx2_char.h>
#include <polarcode/decoding/fastssc_avx_float.h>
#include <polarcode/decoding/scl_avx2_char.h>
#include <polarcode/decoding/scl_avx_float.h>

namespace PolarCode {
namespace Decoding {

Decoder* makeDecoder(size_t blockLength, size_t listSize, const std::vector<unsigned> &frozenBits, int decoder_impl){
  Decoder* dec;
  if(listSize == 1){
		if(decoder_impl == 1){
			dec = new FastSscAvxFloat(blockLength, frozenBits);
		} else{
			dec = new FastSscAvx2Char(blockLength, frozenBits);
		}
  }
  else{
    if(decoder_impl == 1){
      dec = new SclAvxFloat(blockLength, listSize, frozenBits);
    } else{
      dec = new SclAvx2Char(blockLength, listSize, frozenBits);
    }
  }
  return dec;
}

Decoder::Decoder()
	: mErrorDetector(new ErrorDetection::Dummy()),
	  mBlockLength(0),
	  mSystematic(true),
	  mSoftOutput(false),
	  mLlrContainer(nullptr),
	  mBitContainer(nullptr),
	  mOutputContainer(nullptr),
	  mFrozenBits({}),
	  mExternalContainers(false){
}

Decoder::~Decoder() {
	delete mErrorDetector;
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

void Decoder::enableSoftOutput(bool so) {
	mSoftOutput = so;
}

bool Decoder::hasSoftOutput() {
	return mSoftOutput;
}

void Decoder::setErrorDetection(ErrorDetection::Detector *pDetector) {
	delete mErrorDetector;
	mErrorDetector = pDetector;
}

void Decoder::setSignal(const float *pLlr) {
	mLlrContainer->insertLlr(pLlr);
}

void Decoder::setSignal(const char *pLlr) {
	mLlrContainer->insertLlr(pLlr);
}

void Decoder::getDecodedInformationBits(void *pData) {
	memcpy(pData, mOutputContainer, (mBlockLength-mFrozenBits.size()+7)/8);
}

void Decoder::getSoftCodeword(void *pData) {
	mBitContainer->getSoftBits(pData);
}

void Decoder::getSoftInformation(void *pData) {
	mBitContainer->getSoftInformation(pData);
}

bool Decoder::decode_vector(const float *pLlr, void* pData){
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

}//namespace Decoding
}//namespace PolarCode
