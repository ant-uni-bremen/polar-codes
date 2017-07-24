#include <polarcode/encoding/butterfly_avx2_char.h>
#include <polarcode/bitcontainer.h>
#include <polarcode/avxconvenience.h>
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
	mFrozenBits = frozenBits;

	if(mBitContainer != nullptr) delete mBitContainer;
	mBitContainer = new CharContainer(mBlockLength);
}

void ButterflyAvx2Char::encode() {
	try {
		transform();
	} catch(...) {
		std::cerr << "Caught exception in transform." << std::endl;
		exit(1);
	}

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

	int blockCount = (mBlockLength+31)/32;

	int n = log2(mBlockLength);
	int firstLoop = std::min(5, n);

	__m256i Left, Right;

	// Sub-vector-length butterfly operations
	for(int stage = 1; stage <= firstLoop; ++stage) {
		int stageBits = 4<<stage;

		for(int block=0; block<blockCount; ++block) {
			Left = _mm256_load_si256(vBit+block);
			Right = _mm256_subVectorShift_epu8(Left, stageBits);
			Left = _mm256_xor_si256(Left, Right);
			_mm256_store_si256(vBit+block, Left);
		}
	}
	// Cross-vector operands have to be selected differently
	for(int stage = 6; stage<=n; ++stage) {
		int blockShift = 1<<(stage-6);
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

void ButterflyAvx2Char::featureCheck()
		throw (Avx2NotSupportedException) {

	if(!__builtin_cpu_supports("avx2")) {
		throw Avx2NotSupportedException();
	}
}

}//namespace Encoding
}//namespace PolarCode

