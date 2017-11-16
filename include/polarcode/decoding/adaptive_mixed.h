#ifndef PC_DEC_ADPT_MIX_H
#define PC_DEC_ADPT_MIX_H

#include <polarcode/decoding/fastssc_avx2_char.h>
#include <polarcode/decoding/scl_avx_float.h>

namespace PolarCode {
namespace Decoding {

/*!
 * \brief A wrapper class for 8-bit Fast-SSC and 32-bit SCL decoding.
 *
 * The list decoder has a high latency, but achieves up to around 2 dB
 * Eb/N0 more than the fast decoder. As the Fast-SSC decoder indeed is fast,
 * it tries to decode the given signal and only upon failure, the signal is
 * decoded a second time, using the list decoder.
 */
class AdaptiveMixed : public Decoder {
	FastSscAvx2Char *mFastDecoder;
	SclAvxFloat	*mListDecoder;
	size_t mListSize;

public:
	/*!
	 * \brief Create an adaptive decoder.
	 * \param blockLength Block length of the Polar Code.
	 * \param listSize Path limit of the list decoder.
	 * \param frozenBits The set of frozen bits.
	 * \param softOutput Whether to calculate soft output.
	 */
	AdaptiveMixed(size_t blockLength, size_t listSize, const std::vector<unsigned> &frozenBits, bool softOutput = false);
	~AdaptiveMixed();
	bool decode();

	void setSystematic(bool sys);
	void setErrorDetection(ErrorDetection::Detector* pDetector);
	void setSignal(const float *pLlr);
};


}// namespace Decoding
}// namespace PolarCode

#endif// PC_DEC_ADPT_MIX_H