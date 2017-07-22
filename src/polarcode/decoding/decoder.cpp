#include <polarcode/decoding/decoder.h>

namespace PolarCode {
namespace Decoding {


Decoder::Decoder() {

}

void Decoder::setErrorDetection(ErrorDetection::Detector *ed) {
	mErrorDetector = ed;
}


}//namespace Decoding
}//namespace PolarCode
