#ifndef PC_ENC_BUTTERFLY_AVX2_CHAR_H
#define PC_ENC_BUTTERFLY_AVX2_CHAR_H

#include <polarcode/encoding/encoder.h>

namespace PolarCode {
namespace Encoding {

class Avx2NotSupportedException{};

/*!
 * \brief Complete butterfly transformation with AVX2 char bits.
 *
 * This encoder simply executes the complete butterfly transformation.
 * The AVX2 instruction set allows to encode 32 eight bit values per operand,
 * so this encoder can XOR 32 char bits per operand at once.
 *
 */
class ButterflyAvx2Char : public Encoder {
	void transform();
	void featureCheck() throw (Avx2NotSupportedException);

public:
	ButterflyAvx2Char();

	/*!
	 * \brief Create the butterfly encoder and initialize its parameters.
	 * \param blockLength Number of code bits.
	 * \param frozenBits Set of frozen channel indices.
	 */
	ButterflyAvx2Char(size_t blockLength,
					  const std::set<unsigned> &frozenBits);

	~ButterflyAvx2Char();

	void encode();///< Perform the butterfly transformation.
	void initialize(size_t blockLength,
					const std::set<unsigned> &frozenBits);
};


}//namespace Encoding
}//namespace PolarCode

#endif

