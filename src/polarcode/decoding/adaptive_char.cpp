#include <polarcode/decoding/adaptive_char.h>

namespace PolarCode {
namespace Decoding {


AdaptiveChar::AdaptiveChar
	( size_t blockLength
	, size_t listSize
	, const std::vector<unsigned> &frozenBits)
	: mListSize(listSize)
{
	mBlockLength = blockLength;
	mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
	mExternalContainers = true;

	mFastDecoder = new FastSscAvx2Char(mBlockLength, mFrozenBits);
	mListDecoder = new SclAvx2Char(mBlockLength, mListSize, mFrozenBits);
}

AdaptiveChar::~AdaptiveChar() {
	delete mFastDecoder;
	delete mListDecoder;
}

bool AdaptiveChar::decode() {
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

void AdaptiveChar::setSystematic(bool sys) {
	mFastDecoder->setSystematic(sys);
	mListDecoder->setSystematic(sys);
}

void AdaptiveChar::setErrorDetection(ErrorDetection::Detector* pDetector) {
	mFastDecoder->setErrorDetection(pDetector);
	mListDecoder->setErrorDetection(pDetector);
}

void AdaptiveChar::setSignal(const float *pLlr) {
	mFastDecoder->setSignal(pLlr);
	mListDecoder->setSignal(pLlr);
}



}// namespace Decoding
}// namespace PolarCode
