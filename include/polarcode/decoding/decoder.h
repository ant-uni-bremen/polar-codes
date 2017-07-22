#ifndef PC_DEC_DECODER_H
#define PC_DEC_DECODER_H

#include <cstddef>

#include <polarcode/errordetection/errordetector.h>

namespace PolarCode {
namespace Decoding {

class Decoder {
protected:
	ErrorDetection::Detector* mErrorDetector;
	size_t mBlockLength;
	bool mSystematic;

public:
	Decoder();
	virtual ~Decoder() = 0;
	virtual void decode() = 0;
	void setErrorDetection(ErrorDetection::Detector*);
};

}//namespace Decoding
}//namespace PolarCode

#endif //PC_DEC_DECODER_H
