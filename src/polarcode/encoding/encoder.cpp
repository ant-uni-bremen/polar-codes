#include <polarcode/encoding/encoder.h>

namespace PolarCode {
namespace Encoding {

Encoder::Encoder()
	: mBlockLength(0), mSystematic(true), mBitContainer(nullptr),
	  mFrozenBits(std::vector<unsigned>()){
}

Encoder::~Encoder() {
	if(mBitContainer != nullptr) {
		delete mBitContainer;
	}
}

size_t Encoder::blockLength() {
	return mBlockLength;
}

void Encoder::setSystematic(bool sys) {
	mSystematic = sys;
}

bool Encoder::isSystematic() {
	return mSystematic;
}

void Encoder::setInformation(void *pData) {
	mBitContainer->insertPackedInformationBits(pData);
}

void Encoder::getInformation(void *pData) {
	mBitContainer->getPackedInformationBits(pData);
}

void Encoder::setCodeword(void *pData) {
	mBitContainer->insertPackedBits(pData);
}

void Encoder::getEncodedData(void *pData) {
	mBitContainer->getPackedBits(pData);
}

void Encoder::clearFrozenBits() {
	mBitContainer->resetFrozenBits();
}

}//namespace Encoding
}//namespace PolarCode
