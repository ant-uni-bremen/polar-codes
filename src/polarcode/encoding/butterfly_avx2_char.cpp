#include <polarcode/encoding/butterfly_avx2_char.h>
#include <polarcode/encoding/butterfly_avx2.h>
#include <polarcode/bitcontainer.h>
#include <cmath>
#include <iostream>


namespace PolarCode {
namespace Encoding {


ButterflyAvx2Char::ButterflyAvx2Char() {
	featureCheck();
}

ButterflyAvx2Char::ButterflyAvx2Char(
		size_t blockLength,
		const std::vector<unsigned> &frozenBits) {
	featureCheck();
	initialize(blockLength, frozenBits);
}

ButterflyAvx2Char::~ButterflyAvx2Char() {
}

void ButterflyAvx2Char::initialize(
		size_t blockLength,
		const std::vector<unsigned> &frozenBits) {
	mBlockLength = blockLength;
	mFrozenBits = frozenBits;

	if(mBitContainer != nullptr) delete mBitContainer;
	mBitContainer = new CharContainer(mBlockLength, mFrozenBits);
}

void ButterflyAvx2Char::encode() {
	transform();

	if(mSystematic) {
		mBitContainer->resetFrozenBits();
		transform();
	}
}

void ButterflyAvx2Char::transform() {
	__m256i *vBit = reinterpret_cast<__m256i*>(
					dynamic_cast<CharContainer*>(
						mBitContainer
					)->data()
				);
	int n = __builtin_ctz(mBlockLength);//log2() on powers of 2

	for(int stage = 0; stage < n; ++stage) {
		ButterflyAvx2CharTransform(vBit, mBlockLength, stage);
	}
}

void ButterflyAvx2Char::featureCheck() {

	if(!featureCheckAvx2()) {
		throw Avx2NotSupportedException();
	}
}


}//namespace Encoding
}//namespace PolarCode

