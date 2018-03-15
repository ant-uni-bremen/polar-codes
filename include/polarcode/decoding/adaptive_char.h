#ifndef PC_DEC_ADPT_CHAR_H
#define PC_DEC_ADPT_CHAR_H

#include <polarcode/decoding/fastssc_fip_char.h>
#include <polarcode/decoding/scl_fip_char.h>

namespace PolarCode {
namespace Decoding {

/*!
 * \brief A wrapper class for Fast-SSC and SCL decoding, in case the former fails.
 *
 * The list decoder has a high latency, but achieves up to around 2 dB
 * Eb/N0 more than the fast decoder. As the Fast-SSC decoder indeed is fast,
 * it tries to decode the given signal and only upon failure, the signal is
 * decoded a second time, using the list decoder.
 */
class AdaptiveChar : public Decoder {
	FastSscFipChar *mFastDecoder;
	SclFipChar	*mListDecoder;
	size_t mListSize;

public:
	/*!
	 * \brief Create an adaptive decoder.
	 * \param blockLength Block length of the Polar Code.
	 * \param listSize Path limit of the list decoder.
	 * \param frozenBits The set of frozen bits.
	 */
	AdaptiveChar(size_t blockLength, size_t listSize, const std::vector<unsigned> &frozenBits);
	~AdaptiveChar();
	bool decode();

	void setSystematic(bool sys);
	void setErrorDetection(ErrorDetection::Detector* pDetector);
	void setSignal(const float *pLlr);
};


}// namespace Decoding
}// namespace PolarCode

#endif// PC_DEC_ADPT_CHAR_H
