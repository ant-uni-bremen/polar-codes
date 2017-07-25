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


}//namespace Decoding
}//namespace PolarCode
