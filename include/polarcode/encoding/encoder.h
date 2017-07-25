#ifndef PC_ENC_ENCODER_H
#define PC_ENC_ENCODER_H

#include <cstddef>
#include <set>

#include <polarcode/bitcontainer.h>

namespace PolarCode {
namespace Encoding {

/*!
 * \brief The skeleton-class for encoders
 */
class Encoder {
protected:
	size_t mBlockLength;           ///< Block length of the Polar Code
	bool mSystematic;              ///< Whether to use systematic coding
	BitContainer *mBitContainer;   ///< Internal bit memory
	std::set<unsigned> mFrozenBits; ///< Indices for frozen bits


public:
	Encoder();
	virtual ~Encoder();
	virtual void encode() = 0;///< Execute the encoding algorithm.

	/*!
	 * \brief Set the encoder's parameters.
	 * \param blockLength Number of code bits.
	 * \param frozenBits A set of frozen channel indices.
	 */
	virtual void initialize(size_t blockLength,
							const std::set<unsigned> &frozenBits) = 0;

	/*!
	 * \brief Explicitly call setSystematic(false); to use
	 *        non-systematic coding.
	 *
	 * This function serves the purpose of explicitly deactivating the
	 * default systematic coding setting via setSystematic(false),
	 * or re-activating it lateron.
	 * \param sys Whether coding should be systematic.
	 */
	void setSystematic(bool sys);

	/*!
	 * \brief Copies packed information bits into encoder's memory.
	 * \param pData Pointer to memory location of information bytes.
	 */
	void setInformation(void *pData);

	/*!
	 * \brief Writes packed code bits into memory.
	 * \param pData Memory location for encoded data. Must be pre-allocated.
	 */
	void getEncodedData(void *pData);
};

}//namespace Encoding
}//namespace PolarCode

#endif //PC_ENC_ENCODER_H
