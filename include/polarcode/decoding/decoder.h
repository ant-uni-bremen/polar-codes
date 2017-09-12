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
	bool mSoftOutput;///< Whether to calculate soft output bits
	BitContainer *mLlrContainer;///< Soft-input container
	BitContainer *mBitContainer;///< Output bit container
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
	 * \brief Query codeword block Length
	 */
	size_t blockLength();

	/*!
	* \brief Query infoword Length
	*/
	size_t infoLength();

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
	* \brief Query if Decoder produces systematic codeword.
	*
	* Check if the code is systematic.
	*/
	bool isSystematic();

	/*!
	 * \brief Activate soft output decoding. This slows down the bit combination
	 *       step.
	 */
	void enableSoftOutput(bool);

	/*!
	 * \brief Query if the decoder will provide soft output bits.
	 * \return True, if soft output was enabled.
	 */
	bool hasSoftOutput();

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

	/*!
	 * \brief Copy corrected LLR-values into pData.
	 * \param pData Signed 8-bit memory with at least blockLength() bytes allocated.
	 */
	void getSoftCodeword(void *pData);

	/*!
	 * \brief Copy corrected LLR-values of information bits into pData.
	 * \param pData Signed 8-bit memory with at least infoLength() bytes allocated.
	 */
	void getSoftInformation(void *pData);
};

/*!
 * \brief Get Pointer to newly created PolarDecoder impl. Fast or List Decoders.
 * \param blockLength size of a polar codeword
 * \param listSize if '1' FastSSC Decoder is returned. Else: SCL Decoder
 * \param frozenBits positions of frozen bits ordered in ascending order.
 */
  Decoder* makeDecoder(size_t blockLength, size_t listSize, const std::vector<unsigned> &frozenBits);
}//namespace Decoding
}//namespace PolarCode

#endif //PC_DEC_DECODER_H
