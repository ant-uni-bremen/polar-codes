#ifndef PC_DEC_DECODER_H
#define PC_DEC_DECODER_H

#include <cstddef>

#include <polarcode/bitcontainer.h>
#include <polarcode/errordetection/errordetector.h>

namespace PolarCode {
namespace Decoding {

/*!
 * \brief The Decoder skeleton-class.
 *
 *
 */
class Decoder {
protected:
	ErrorDetection::Detector* mErrorDetector;///< Error detecting object
	size_t mBlockLength;///< Length of the Polar Code
	bool mSystematic;///< Whether to use systematic coding
	CharContainer *mLlrContainer;///< Soft-input container
	CharContainer *mBitContainer;///< Hard-output bit container
	unsigned char *mOutputContainer;///< Final data container, gets filled for error detection
	std::vector<unsigned> mFrozenBits; ///< Indices for frozen bits

public:
	Decoder();
	virtual ~Decoder();

	/*!
	 * \brief Decode the received signal.
	 * \return True, if no errors detected after decoding.
	 */
	virtual bool decode() = 0;

	/*!
	 * \brief Set the decoder's parameters.
	 * \param blockLength Number of code bits.
	 * \param frozenBits A set of frozen channel indices.
	 */
	virtual void initialize(size_t blockLength,
							const std::vector<unsigned> &frozenBits) = 0;
	/*!
	 * \brief Set an error detection scheme.
	 * \param pDetector Pointer to an error detecting object. Ownership is taken.
	 */
	void setErrorDetection(ErrorDetection::Detector* pDetector);

	/*!
	 * \brief Copy the received signal into decoder's memory.
	 * \param pLlr Pointer to single precision floating point LLRs.
	 */
	void setSignal(const float *pLlr);

	/*!
	 * \brief Copy the received signal into decoder's memory.
	 * \param pLlr Pointer to eight-bit integer LLRs.
	 */
	void setSignal(const char  *pLlr);

	/*!
	 * \brief Write the information bits from the decoded code word into _pData_.
	 * \param pData Pointer to destination memory of decoded information.
	 */
	void getDecodedInformationBits(void *pData);
};

}//namespace Decoding
}//namespace PolarCode

#endif //PC_DEC_DECODER_H
