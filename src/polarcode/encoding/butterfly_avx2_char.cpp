#include <polarcode/encoding/butterfly_avx2_char.h>
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
		const std::set<unsigned> &frozenBits) {
	featureCheck();
	initialize(blockLength, frozenBits);
}

ButterflyAvx2Char::~ButterflyAvx2Char() {
}

void ButterflyAvx2Char::initialize(
		size_t blockLength,
		const std::set<unsigned> &frozenBits) {
	mBlockLength = blockLength;
	//mFrozenBits = frozenBits;
	mFrozenBits.clear();
	mFrozenBits.insert(frozenBits.begin(), frozenBits.end());

	if(mBitContainer != nullptr) delete mBitContainer;
	mBitContainer = new CharContainer(mBlockLength);
}

void ButterflyAvx2Char::encode() {
	transform();

	if(mSystematic) {
		mBitContainer->resetFrozenBits(mFrozenBits);
		transform();
	}
}

void ButterflyAvx2Char::transform() {
	__m256i *vBit = reinterpret_cast<__m256i*>(
					dynamic_cast<CharContainer*>(
						mBitContainer
					)->data()
				);
	int n = log2(mBlockLength);

	for(int stage = 0; stage < n; ++stage) {
		ButterflyAvx2CharTransform(vBit, mBlockLength, stage);
	}
}

void ButterflyAvx2CharTransform(__m256i* bitVector, size_t blockLength, int stage) {
	int blockCount = (blockLength+31)/32;

	if(stage < 5) {
		ButterflyAvx2CharTransformSubVector(bitVector, stage, blockCount);
	} else {
		ButterflyAvx2CharTransformCrossVector(bitVector, stage, blockCount);
	}
}

void ButterflyAvx2CharTransformSubVector(__m256i* bitVector, int stage, int blockCount) {
	__m256i Left, Right;
	int stageBits = 8<<stage;

	for(int block=0; block<blockCount; ++block) {
		Left = _mm256_load_si256(bitVector+block);
		Right = _mm256_subVectorShift_epu8(Left, stageBits);
		Left = _mm256_xor_si256(Left, Right);
		_mm256_store_si256(bitVector+block, Left);
	}

}

void ButterflyAvx2CharTransformCrossVector(__m256i* bitVectors, int stage, int blockCount) {
	__m256i Left, Right;
	int blockShift = 1<<(stage-5);
	int blockJump = blockShift*2;
	for(int group=0; group<blockCount; group+=blockJump) {
		for(int block=0; block < blockShift; block += 1) {
			Left = _mm256_load_si256(bitVectors+group+block);
			Right = _mm256_load_si256(bitVectors+group+block+blockShift);
			Left = _mm256_xor_si256(Left, Right);
			_mm256_store_si256(bitVectors+group+block, Left);
		}
	}
}

void ButterflyAvx2Char::featureCheck()
		throw (Avx2NotSupportedException) {

	if(!featureCheckAvx2()) {
		throw Avx2NotSupportedException();
	}
}


}//namespace Encoding
}//namespace PolarCode

