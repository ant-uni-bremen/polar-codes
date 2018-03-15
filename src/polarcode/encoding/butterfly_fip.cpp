#include <polarcode/encoding/butterfly_fip.h>

namespace PolarCode {
namespace Encoding {


void ButterflyFipTransform(fipv* bitVector, size_t blockLength, int stage, const int bitsPerExpandedBit) {
	const int expandedBitsPerVector = BITSPERVECTOR / bitsPerExpandedBit;
	const int vectorStage = __builtin_ctz(expandedBitsPerVector);//log2()
	int blockCount = (blockLength + expandedBitsPerVector - 1) / expandedBitsPerVector;

	if(stage < vectorStage) {
		ButterflyFipTransformSubVector(bitVector, stage, blockCount, bitsPerExpandedBit);
	} else {
		ButterflyFipTransformCrossVector(bitVector, stage, blockCount, vectorStage);
	}
}

void ButterflyFipTransformSubVector(fipv* bitVector, int stage, int blockCount, const int bitsPerExpandedBit) {
	fipv Left, Right;
	int stageBits = bitsPerExpandedBit << stage;

	for(int block = 0; block < blockCount; ++block) {
		Left = fi_load(bitVector + block);
		Right = subVectorShift_epu8(Left, stageBits);
		Left = fi_xor(Left, Right);
		fi_store(bitVector + block, Left);
	}

}

void ButterflyFipTransformCrossVector(fipv* bitVectors, int stage, int blockCount, const int logsize) {
	fipv Left, Right;
	int blockShift = 1 << (stage - logsize);
	int blockJump = blockShift * 2;
	for(int group = 0; group < blockCount; group += blockJump) {
		for(int block = 0; block < blockShift; block += 1) {
			Left = fi_load(bitVectors + group + block);
			Right = fi_load(bitVectors + group + block + blockShift);
			Left = fi_xor(Left, Right);
			fi_store(bitVectors + group + block, Left);
		}
	}
}

void ButterflyFipCharTransform(fipv* bitVector, size_t blockLength, int stage) {
	ButterflyFipTransform(bitVector, blockLength, stage, 8);
}

void ButterflyFipPackedTransform(fipv* bitVector, size_t blockLength, int stage) {
	ButterflyFipTransform(bitVector, blockLength, stage, 1);
}

}// namespace PolarCode
}// namespace Encoding
