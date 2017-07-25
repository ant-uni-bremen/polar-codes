#include <polarcode/polarcode.h>

namespace PolarCode {

PolarCoder::PolarCoder(Encoding::Encoder *encoder, Decoding::Decoder *decoder)
	: mEncoder(encoder), mDecoder(decoder) {
}

}//namespace PolarCode
