#include <polarcode/decoding/decoder.h>
#include <polarcode/errordetection/dummy.h>
#include <cstring>
#include <polarcode/decoding/fastssc_avx2_char.h>
#include <polarcode/decoding/scl_avx2_char.h>

namespace PolarCode {
namespace Decoding {

Decoder* makeDecoder(size_t blockLength, size_t listSize, const std::vector<unsigned> &frozenBits){
  Decoder* dec;
  if(listSize == 1){
    dec = new FastSscAvx2Char(blockLength, frozenBits);
  }
  else{
    dec = new SclAvx2Char(blockLength, listSize, frozenBits);
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
	  mFrozenBits({}) {
}

Decoder::~Decoder() {
	delete mErrorDetector;
	if(mLlrContainer) delete mLlrContainer;
	if(mBitContainer) delete mBitContainer;
	if(mOutputContainer) delete [] mOutputContainer;
}

size_t Decoder::blockLength() {
	return mBlockLength;
}

size_t Decoder::infoLength() {
	return mBlockLength - mFrozenBits.size();
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

}//namespace Decoding
}//namespace PolarCode
