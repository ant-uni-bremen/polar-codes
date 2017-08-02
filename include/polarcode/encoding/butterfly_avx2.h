#ifndef PC_ENC_BUTTERFLY_AVX2_H
#define PC_ENC_BUTTERFLY_AVX2_H

#include <polarcode/avxconvenience.h>

namespace PolarCode {
namespace Encoding {

void ButterflyAvx2Transform(__m256i* bitVector, size_t blockLength, int stage, const int bitsPerExpandedBit);
void ButterflyAvx2TransformSubVector(__m256i* bitVector, int stage, int blockCount, const int bitsPerExpandedBit);
void ButterflyAvx2TransformCrossVector(__m256i* bitVectors, int stage, int blockCount, const int logsize);


void ButterflyAvx2CharTransform(__m256i* bitVector, size_t blockLength, int stage);
void ButterflyAvx2PackedTransform(__m256i* bitVector, size_t blockLength, int stage);

}// namespace Encoding
}// namespace PolarCode
#endif
