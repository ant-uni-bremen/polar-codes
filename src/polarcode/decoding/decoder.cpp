#include <polarcode/decoding/decoder.h>
#include <polarcode/errordetection/dummy.h>
#include <cstring>

namespace PolarCode {
namespace Decoding {


Decoder::Decoder()
	: mErrorDetector(new ErrorDetection::Dummy()),
	  mBlockLength(0),
	  mSystematic(true),
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

}//namespace Decoding
}//namespace PolarCode
