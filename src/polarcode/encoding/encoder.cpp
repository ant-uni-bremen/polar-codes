#include <polarcode/encoding/encoder.h>

namespace PolarCode {
namespace Encoding {

Encoder::Encoder()
	: mBlockLength(0), mSystematic(true), mBitContainer(nullptr),
	  mFrozenBits(std::set<unsigned>()){
}

Encoder::~Encoder() {
	if(mBitContainer != nullptr) {
		delete mBitContainer;
	}
}

void Encoder::setSystematic(bool sys) {
	mSystematic = sys;
}

void Encoder::setInformation(void *pData) {
	mBitContainer->insertPackedInformationBits(pData);
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
