#include <polarcode/decoding/decoder.h>
#include <polarcode/errordetection/dummy.h>

namespace PolarCode {
namespace Decoding {


Decoder::Decoder()
	: mErrorDetector(new ErrorDetection::Dummy()),
	  mSystematic(true) {

}

Decoder::~Decoder() {
	delete mErrorDetector;
}

void Decoder::setErrorDetection(ErrorDetection::Detector *pDetector) {
	delete mErrorDetector;
	mErrorDetector = pDetector;
}

void Decoder::setSignal(const float *pLlr) {
	mBitContainer->insertLlr(pLlr);
}

void Decoder::setSignal(const char *pLlr) {
	mBitContainer->insertLlr(pLlr);
}


}//namespace Decoding
}//namespace PolarCode
