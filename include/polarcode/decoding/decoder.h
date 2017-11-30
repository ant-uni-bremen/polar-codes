#ifndef PC_DEC_DECODER_H
#define PC_DEC_DECODER_H

#include <cstddef>
#include <string>

#include <polarcode/bitcontainer.h>
#include <polarcode/errordetection/errordetector.h>

namespace PolarCode {
namespace Decoding {

struct CodingScheme {
	unsigned int blockLength, infoLength;
	std::vector<unsigned> frozenBits;
	bool systematic;
	//CRC?
};

//Filled by fixed decoder factory
//Public access allows encoders to be designed corresponding to a given coding
// scheme.
extern std::vector<CodingScheme> codeRegistry;

/*!
 * \brief The Decoder skeleton-class.
 *
 *
 */
class Decoder {
private:
    size_t mDecoderDuration;

protected:
	ErrorDetection::Detector* mErrorDetector;///< Error detecting object
	size_t mBlockLength;///< Length of the Polar Code
	bool mSystematic;///< Whether to use systematic coding
	BitContainer *mLlrContainer;///< Soft-input container
	BitContainer *mBitContainer;///< (optionally soft-) Output bit container
	unsigned char *mOutputContainer;///< Final data container, gets filled for error detection
	std::vector<unsigned> mFrozenBits; ///< Indices for frozen bits
	bool mExternalContainers;///< On destruction, do not delete containers

public:
	Decoder();
	virtual ~Decoder();

	/*!
	 * \brief Decode the received signal.
	 * \return True, if no errors detected after decoding.
	 */
	virtual bool decode() = 0;

	/*!
	 * \brief Decode float vector
	 * \return True, if no errors detected after decoding.
	 */
	bool decode_vector(const float *pLlr, void* pData);

	/*!
	 * \brief Decode char vector
	 * \return True, if no errors detected after decoding.
	 */
	bool decode_vector(const char *pLlr, void* pData);

	/*!
	* \brief Decoder duration
	* \return Number of ticks in nanoseconds for last decoder call.
	*/
	size_t duration_ns(){ return mDecoderDuration;}

	/*!
	 * \brief Set the decoder's parameters.
	 * \param blockLength Number of code bits.
	 * \param frozenBits A set of frozen channel indices.
	 */
	virtual void initialize(size_t blockLength,
							const std::vector<unsigned> &frozenBits);


	/*!
	* \brief Query frozenBits
	*/
	std::vector<unsigned> frozenBits(){ return mFrozenBits;}

	/*!
	 * \brief Query codeword block Length
	 */
	size_t blockLength();

	/*!
	* \brief Query infoword Length
	*/
	size_t infoLength();

	BitContainer* inputContainer(); ///< Get direct pointer to mLlrContainer
	BitContainer* outputContainer();///< Get direct pointer to mBitContainer
	unsigned char* packedOutput();  ///< Get direct pointer to mOutputContainer

	/*!
	* \brief Explicitly call setSystematic(false); to use
	*        non-systematic coding.
	*
	* This function serves the purpose of explicitly deactivating the
	* default systematic coding setting via setSystematic(false),
	* or re-activating it lateron.
	* \param sys Whether coding should be systematic.
	*/
	virtual void setSystematic(bool sys);

	/*!
	* \brief Query if Decoder produces systematic codeword.
	*
	* Check if the code is systematic.
	*/
	bool isSystematic();

	/*!
	 * \brief Set an error detection scheme.
	 * \param pDetector Pointer to an error detecting object.
	 */
	virtual void setErrorDetection(ErrorDetection::Detector* pDetector);

	/*!
	 * \brief Copy the received signal into decoder's memory.
	 * \param pLlr Pointer to single precision floating point LLRs.
	 */
	virtual void setSignal(const float *pLlr);

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
  Decoder* makeDecoder(size_t blockLength, size_t listSize, const std::vector<unsigned> &frozenBits, int decoder_impl=0);
}//namespace Decoding
}//namespace PolarCode

#endif //PC_DEC_DECODER_H
