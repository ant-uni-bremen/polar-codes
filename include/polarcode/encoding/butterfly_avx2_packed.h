#ifndef PC_ENC_BUTTERFLY_AVX2_PACKED_H
#define PC_ENC_BUTTERFLY_AVX2_PACKED_H

#include <polarcode/encoding/encoder.h>
#include <polarcode/avxconvenience.h>

namespace PolarCode {
namespace Encoding {

void ButterflyAvx2PackedTransform(__m256i* bitVector, size_t blockLength, int stage);
void ButterflyAvx2PackedTransformSubVector(__m256i* bitVector, int stage, int blockCount);
void ButterflyAvx2PackedTransformCrossVector(__m256i* bitVectors, int stage, int blockCount);

/*!
 * \brief Complete butterfly transformation with AVX2 operations.
 *
 * This encoder simply executes the complete butterfly transformation.
 * The AVX2 instruction set allows to encode 256 bit values per operand,
 * so this encoder can XOR 256 bits per operand at once.
 *
 */
class ButterflyAvx2Packed : public Encoder {
	void transform();
	void featureCheck() throw (Avx2NotSupportedException);

public:
	ButterflyAvx2Packed();

	/*!
	 * \brief Create the butterfly encoder without frozen bits.
	 * \param blockLength Number of code bits.
	 */
	ButterflyAvx2Packed(size_t blockLength);

	/*!
	 * \brief Create the butterfly encoder and initialize its parameters.
	 * \param blockLength Number of code bits.
	 * \param frozenBits Set of frozen channel indices.
	 */
	ButterflyAvx2Packed(size_t blockLength,
					  const std::vector<unsigned> &frozenBits);

	~ButterflyAvx2Packed();

	void encode();///< Perform the butterfly transformation.
	void initialize(size_t blockLength,
					const std::vector<unsigned> &frozenBits);
};


}//namespace Encoding
}//namespace PolarCode

#endif // PC_ENC_BUTTERFLY_AVX2_PACKED_H

