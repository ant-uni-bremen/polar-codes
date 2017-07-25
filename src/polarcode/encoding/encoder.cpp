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
	mBitContainer->insertPackedInformationBits(pData, mFrozenBits);
}

void Encoder::getEncodedData(void *pData) {
	mBitContainer->getPackedBits(pData);
}

}//namespace Encoding
}//namespace PolarCode
