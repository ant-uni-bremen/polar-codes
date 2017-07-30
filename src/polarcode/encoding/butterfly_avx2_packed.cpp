#include <polarcode/encoding/butterfly_avx2_packed.h>
#include <polarcode/bitcontainer.h>
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

ButterflyAvx2Packed::ButterflyAvx2Packed(
		size_t blockLength,
		const std::set<unsigned> &frozenBits) {
	featureCheck();
	initialize(blockLength, frozenBits);
}

ButterflyAvx2Packed::~ButterflyAvx2Packed() {
}

void ButterflyAvx2Packed::initialize(
		size_t blockLength,
		const std::set<unsigned> &frozenBits) {
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
					)->data()
				);

	int blockCount = (mBlockLength+255)/256;

	int n = log2(mBlockLength);
	int firstLoop = std::min(8, n);

	__m256i Left, Right;

	// Sub-vector-length butterfly operations
	for(int stage = 1; stage <= firstLoop; ++stage) {
		int stageBits = 1<<(stage-1);

		for(int block=0; block<blockCount; ++block) {
			Left = _mm256_load_si256(vBit+block);
			Right = _mm256_subVectorShift_epu8(Left, stageBits);
			Left = _mm256_xor_si256(Left, Right);
			_mm256_store_si256(vBit+block, Left);
		}
	}
	// Cross-vector operands have to be selected differently
	for(int stage = 9; stage<=n; ++stage) {
		int blockShift = 1<<(stage-9);
		int blockJump = blockShift*2;
		for(int group=0; group<blockCount; group+=blockJump) {
			for(int block=0; block < blockShift; block += 1) {
				Left = _mm256_load_si256(vBit+group+block);
				Right = _mm256_load_si256(vBit+group+block+blockShift);
				Left = _mm256_xor_si256(Left, Right);
				_mm256_store_si256(vBit+group+block, Left);
			}
		}
	}
}

void ButterflyAvx2Packed::featureCheck()
		throw (Avx2NotSupportedException) {

	if(!featureCheckAvx2()) {
		throw Avx2NotSupportedException();
	}
}


}//namespace Encoding
}//namespace PolarCode

