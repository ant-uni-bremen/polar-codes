#include <polarcode/decoding/adaptive_mixed.h>

namespace PolarCode {
namespace Decoding {


AdaptiveMixed::AdaptiveMixed
	( size_t blockLength
	, size_t listSize
	, const std::vector<unsigned> &frozenBits)
	: mListSize(listSize)
{
	mBlockLength = blockLength;
	mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
	mExternalContainers = true;

	mFastDecoder = new FastSscFipChar(mBlockLength, mFrozenBits);
	mListDecoder = new SclAvxFloat(mBlockLength, mListSize, mFrozenBits);
}

AdaptiveMixed::~AdaptiveMixed() {
	delete mFastDecoder;
	delete mListDecoder;
}

bool AdaptiveMixed::decode() {
	bool success = mFastDecoder->decode();
	mOutputContainer = mFastDecoder->packedOutput();
	mBitContainer = mFastDecoder->outputContainer();
	if(!success && mListSize > 1) {
		success = mListDecoder->decode();
		mOutputContainer = mListDecoder->packedOutput();
		mBitContainer = mListDecoder->outputContainer();
	}
	return success;
}

void AdaptiveMixed::setSystematic(bool sys) {
	mFastDecoder->setSystematic(sys);
	mListDecoder->setSystematic(sys);
}

void AdaptiveMixed::setErrorDetection(ErrorDetection::Detector* pDetector) {
	mFastDecoder->setErrorDetection(pDetector);
	mListDecoder->setErrorDetection(pDetector);
}

void AdaptiveMixed::setSignal(const float *pLlr) {
	mFastDecoder->setSignal(pLlr);
	mListDecoder->setSignal(pLlr);
}



}// namespace Decoding
}// namespace PolarCode
