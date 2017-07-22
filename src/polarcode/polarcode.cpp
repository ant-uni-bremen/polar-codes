#include <polarcode/polarcode.h>

namespace PolarCode {

PolarCoder::PolarCoder(Encoding::Encoder *enc, Decoding::Decoder *dec,
					   Construction::Constructor *con)
	: mEncoder(enc), mDecoder(dec), mConstructor(con) {
}

}//namespace PolarCode
