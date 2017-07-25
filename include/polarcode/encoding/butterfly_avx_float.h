#ifndef PC_ENC_BUTTERFLY_AVX_FLOAT_H
#define PC_ENC_BUTTERFLY_AVX_FLOAT_H

#include <polarcode/encoding/encoder.h>

namespace PolarCode {
namespace Encoding {

/*!
 * \brief Complete butterfly transformation with AVX "floating point" bits.
 *
 * This encoder simply executes the complete butterfly transformation.
 * The AVX instruction set uses single precision floating point arithmetic,
 * which is not really well utilizable for polar encoding, as only eight
 * code bits can be hold in a 256-bit register.
 *
 */
class ButterflyAvxFloat : public Encoder {
	void transform();

public:
	ButterflyAvxFloat();

	/*!
	 * \brief Create the butterfly encoder and initialize its parameters.
	 * \param blockLength Number of code bits.
	 * \param frozenBits Set of frozen channel indices.
	 */
	ButterflyAvxFloat(size_t blockLength,
					  const std::set<unsigned> &frozenBits);

	~ButterflyAvxFloat();

	void encode();
	void initialize(size_t blockLength,
					const std::set<unsigned> &frozenBits);
};


}//namespace Encoding
}//namespace PolarCode

#endif
