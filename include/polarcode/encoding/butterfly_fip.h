#ifndef PC_ENC_BUTTERFLY_FIP_H
#define PC_ENC_BUTTERFLY_FIP_H

#include <polarcode/avxconvenience.h>

namespace PolarCode {
namespace Encoding {

void ButterflyFipTransform(fipv* bitVector, size_t blockLength, int stage, const int bitsPerExpandedBit);
void ButterflyFipTransformSubVector(fipv* bitVector, int stage, int blockCount, const int bitsPerExpandedBit);
void ButterflyFipTransformCrossVector(fipv* bitVectors, int stage, int blockCount, const int logsize);


void ButterflyFipCharTransform(fipv* bitVector, size_t blockLength, int stage);
void ButterflyFipPackedTransform(fipv* bitVector, size_t blockLength, int stage);

}// namespace Encoding
}// namespace PolarCode
#endif
