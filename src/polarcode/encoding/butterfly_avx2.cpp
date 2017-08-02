#include <polarcode/encoding/butterfly_avx2.h>

namespace PolarCode {
namespace Encoding {


void ButterflyAvx2Transform(__m256i* bitVector, size_t blockLength, int stage, const int bitsPerExpandedBit) {
	const int expandedBitsPerVector = 256/bitsPerExpandedBit;
	const int vectorStage = __builtin_ctz(expandedBitsPerVector);//log2()
	int blockCount = (blockLength+expandedBitsPerVector-1)/expandedBitsPerVector;

	if(stage < vectorStage) {
		ButterflyAvx2TransformSubVector(bitVector, stage, blockCount, bitsPerExpandedBit);
	} else {
		ButterflyAvx2TransformCrossVector(bitVector, stage, blockCount, vectorStage);
	}
}

void ButterflyAvx2TransformSubVector(__m256i* bitVector, int stage, int blockCount, const int bitsPerExpandedBit) {
	__m256i Left, Right;
	int stageBits = bitsPerExpandedBit<<stage;

	for(int block=0; block<blockCount; ++block) {
		Left = _mm256_load_si256(bitVector+block);
		Right = _mm256_subVectorShift_epu8(Left, stageBits);
		Left = _mm256_xor_si256(Left, Right);
		_mm256_store_si256(bitVector+block, Left);
	}

}

void ButterflyAvx2TransformCrossVector(__m256i* bitVectors, int stage, int blockCount, const int logsize) {
	__m256i Left, Right;
	int blockShift = 1<<(stage-logsize);
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

void ButterflyAvx2CharTransform(__m256i* bitVector, size_t blockLength, int stage) {
	ButterflyAvx2Transform(bitVector, blockLength, stage, 8);
}

void ButterflyAvx2PackedTransform(__m256i* bitVector, size_t blockLength, int stage) {
	ButterflyAvx2Transform(bitVector, blockLength, stage, 1);
}

}// namespace PolarCode
}// namespace Encoding
