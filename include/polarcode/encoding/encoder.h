#ifndef PC_ENC_ENCODER_H
#define PC_ENC_ENCODER_H

#include <cstddef>
#include <vector>

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
	std::vector<unsigned> mFrozenBits; ///< Indices for frozen bits

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
							const std::vector<unsigned> &frozenBits) = 0;

	/*!
	* \brief Query codeword block Length
	*/
	size_t blockLength();

	/*!
	* \brief Query frozenBits
	*/
	std::vector<unsigned> frozenBits(){ return mFrozenBits;}

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
	* \brief Query if Encoder produces systematic codeword.
	*
	* Check if the code is systematic.
	*/
	bool isSystematic();

	/*!
	 * \brief Copies packed information bits into encoder's memory.
	 * \param pData Pointer to memory location of information bytes.
	 */
	void setInformation(void *pData);

	/*!
	 * \brief Copy packed information bits from encoder.
	 * \param pData Pointer to memory location for information bits.
	 */
	void getInformation(void *pData);

	/*!
	 * \brief Insert packed bits.
	 * \param pData Pointer to memory location of packed bits.
	 */
	void setCodeword(void *pData);

	/*!
	 * \brief Writes packed code bits into memory.
	 * \param pData Memory location for encoded data. Must be pre-allocated.
	 */
	void getEncodedData(void *pData);

	/*!
	 * \brief Set all frozen bits to 0. This is needed by the recursive encoder.
	 */
	void clearFrozenBits();
};

}//namespace Encoding
}//namespace PolarCode

#endif //PC_ENC_ENCODER_H
