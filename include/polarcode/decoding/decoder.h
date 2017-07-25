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
	BitContainer *mBitContainer;///< Soft-input / hard-output bit container

public:
	Decoder();
	virtual ~Decoder();

	/*!
	 * \brief Decode the received signal.
	 */
	virtual void decode() = 0;

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
};

}//namespace Decoding
}//namespace PolarCode

#endif //PC_DEC_DECODER_H
