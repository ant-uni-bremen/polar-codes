#include <polarcode/decoding/adaptive_float.h>

namespace PolarCode {
namespace Decoding {


AdaptiveFloat::AdaptiveFloat
	( size_t blockLength
	, size_t listSize
	, const std::vector<unsigned> &frozenBits
	, bool softOutput)
{
	mBlockLength = blockLength;
	mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
	mSoftOutput = softOutput;
	mExternalContainers = true;

	mFastDecoder = new FastSscAvxFloat(mBlockLength, mFrozenBits);
	mListDecoder = new SclAvxFloat(mBlockLength, listSize, mFrozenBits, mSoftOutput);
}

AdaptiveFloat::~AdaptiveFloat() {
	delete mFastDecoder;
	delete mListDecoder;
}

bool AdaptiveFloat::decode() {
	bool success;
	if(mFastDecoder->decode()) {
		mOutputContainer = mFastDecoder->packedOutput();
		mBitContainer = mFastDecoder->outputContainer();
		success = true;
	} else {
		success = mListDecoder->decode();
		mOutputContainer = mListDecoder->packedOutput();
		mBitContainer = mListDecoder->outputContainer();
	}
	return success;
}

void AdaptiveFloat::setSystematic(bool sys) {
	mFastDecoder->setSystematic(sys);
	mListDecoder->setSystematic(sys);
}

void AdaptiveFloat::setErrorDetection(ErrorDetection::Detector* pDetector) {
	mFastDecoder->setErrorDetection(pDetector->clone());
	mListDecoder->setErrorDetection(pDetector);
}

void AdaptiveFloat::setSignal(const float *pLlr) {
	mFastDecoder->setSignal(pLlr);
	mListDecoder->setSignal(pLlr);
}



}// namespace Decoding
}// namespace PolarCode
