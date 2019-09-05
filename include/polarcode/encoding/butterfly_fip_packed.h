#ifndef PC_ENC_BUTTERFLY_FIP_PACKED_H
#define PC_ENC_BUTTERFLY_FIP_PACKED_H

#include <polarcode/encoding/encoder.h>

namespace PolarCode {
namespace Encoding {

/*!
 * \brief Complete butterfly transformation with AVX2 operations.
 *
 * This encoder simply executes the complete butterfly transformation.
 * The AVX2 instruction set allows to encode 256 bit values per operand,
 * so this encoder can XOR 256 bits per operand at once.
 *
 */
class ButterflyFipPacked : public Encoder {
	void transform();

public:
	ButterflyFipPacked();

	/*!
	 * \brief Create the butterfly encoder without frozen bits.
	 * \param blockLength Number of code bits.
	 */
	ButterflyFipPacked(size_t blockLength);

	/*!
	 * \brief Create the butterfly encoder and initialize its parameters.
	 * \param blockLength Number of code bits.
	 * \param frozenBits Set of frozen channel indices.
	 */
	ButterflyFipPacked(size_t blockLength,
					  const std::vector<unsigned> &frozenBits);

	~ButterflyFipPacked();

	void encode();///< Perform the butterfly transformation.
	void initialize(size_t blockLength,
					const std::vector<unsigned> &frozenBits);
};


}//namespace Encoding
}//namespace PolarCode

#endif // PC_ENC_BUTTERFLY_FIP_PACKED_H

