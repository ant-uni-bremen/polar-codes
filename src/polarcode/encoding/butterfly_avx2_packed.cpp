#include <polarcode/encoding/butterfly_avx2_packed.h>
#include <polarcode/encoding/butterfly_avx2.h>
#include <cmath>
#include <iostream>


namespace PolarCode {
namespace Encoding {


ButterflyAvx2Packed::ButterflyAvx2Packed() {
}

ButterflyAvx2Packed::ButterflyAvx2Packed(size_t blockLength) {
	initialize(blockLength, {});
}

ButterflyAvx2Packed::ButterflyAvx2Packed(size_t blockLength,
		const std::vector<unsigned> &frozenBits) {
	initialize(blockLength, frozenBits);
}

ButterflyAvx2Packed::~ButterflyAvx2Packed() {
}

void ButterflyAvx2Packed::initialize(size_t blockLength,
		const std::vector<unsigned> &frozenBits) {
	mBlockLength = blockLength;
	mFrozenBits.assign(frozenBits.begin(), frozenBits.end());

	if(mBitContainer != nullptr) delete mBitContainer;
	mBitContainer = new PackedContainer(mBlockLength, mFrozenBits);
}

void ButterflyAvx2Packed::encode() {
	mErrorDetector->generate(xmInputData, (mBlockLength - mFrozenBits.size()) / 8);
	mBitContainer->insertPackedInformationBits(xmInputData);

	transform();

	if(mSystematic) {
		mBitContainer->resetFrozenBits();
		transform();
	}
}

void ButterflyAvx2Packed::transform() {
	__m256i *vBit = reinterpret_cast<__m256i*>(
						dynamic_cast<PackedContainer*>(
							mBitContainer
						)->data());
	int n = __builtin_ctz(mBlockLength);//log2() on powers of 2

	for(int stage = 0; stage<n; ++stage) {
		ButterflyAvx2PackedTransform(vBit, mBlockLength, stage);
	}
}


}//namespace Encoding
}//namespace PolarCode

