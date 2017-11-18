#include <polarcode/encoding/encoder.h>
#include <polarcode/errordetection/dummy.h>

namespace PolarCode {
namespace Encoding {

Encoder::Encoder()
	: mErrorDetector(&ErrorDetection::globalDummyDetector)
	, mBlockLength(0)
	, mSystematic(true)
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
}

void Encoder::getEncodedData(void *pData) {
	mBitContainer->getPackedBits(pData);
}

void Encoder::clearFrozenBits() {
	mBitContainer->resetFrozenBits();
}

}//namespace Encoding
}//namespace PolarCode
