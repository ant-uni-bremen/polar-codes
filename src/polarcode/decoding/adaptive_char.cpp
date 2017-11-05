#include <polarcode/decoding/adaptive_char.h>

namespace PolarCode {
namespace Decoding {


AdaptiveChar::AdaptiveChar
	( size_t blockLength
	, size_t listSize
	, const std::vector<unsigned> &frozenBits
	, bool softOutput)
{
	mBlockLength = blockLength;
	mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
	mSoftOutput = softOutput;
	mExternalContainers = true;

	mFastDecoder = new FastSscAvx2Char(mBlockLength, mFrozenBits);
	mListDecoder = new SclAvx2Char(mBlockLength, listSize, mFrozenBits, mSoftOutput);
}

AdaptiveChar::~AdaptiveChar() {
	delete mFastDecoder;
	delete mListDecoder;
}

bool AdaptiveChar::decode() {
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

void AdaptiveChar::setSystematic(bool sys) {
	mFastDecoder->setSystematic(sys);
	mListDecoder->setSystematic(sys);
}

void AdaptiveChar::setErrorDetection(ErrorDetection::Detector* pDetector) {
	mFastDecoder->setErrorDetection(pDetector->clone());
	mListDecoder->setErrorDetection(pDetector);
}

void AdaptiveChar::setSignal(const float *pLlr) {
	mFastDecoder->setSignal(pLlr);
	mListDecoder->setSignal(pLlr);
}



}// namespace Decoding
}// namespace PolarCode
