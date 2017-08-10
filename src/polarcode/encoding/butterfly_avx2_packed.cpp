#include <polarcode/encoding/butterfly_avx2_packed.h>
#include <polarcode/encoding/butterfly_avx2.h>
#include <cmath>
#include <iostream>


namespace PolarCode {
namespace Encoding {


ButterflyAvx2Packed::ButterflyAvx2Packed() {
	featureCheck();
}

ButterflyAvx2Packed::ButterflyAvx2Packed(size_t blockLength) {
	featureCheck();
	initialize(blockLength, {});
}

ButterflyAvx2Packed::ButterflyAvx2Packed(size_t blockLength,
		const std::vector<unsigned> &frozenBits) {
	featureCheck();
	initialize(blockLength, frozenBits);
}

ButterflyAvx2Packed::~ButterflyAvx2Packed() {
}

void ButterflyAvx2Packed::initialize(size_t blockLength,
		const std::vector<unsigned> &frozenBits) {
	mBlockLength = blockLength;
	mFrozenBits = frozenBits;

	if(mBitContainer != nullptr) delete mBitContainer;
	mBitContainer = new PackedContainer(mBlockLength, mFrozenBits);
}

void ButterflyAvx2Packed::encode() {
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

void ButterflyAvx2Packed::featureCheck() {

	if(!featureCheckAvx2()) {
		throw Avx2NotSupportedException();
	}
}


}//namespace Encoding
}//namespace PolarCode

