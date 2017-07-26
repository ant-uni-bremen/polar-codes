#include <polarcode/polarcode.h>

namespace PolarCode {

void splitFrozenBits(
		const std::set<unsigned> &source,
		size_t subBlockLength,
		std::set<unsigned> &left,
		std::set<unsigned> &right) {
	left.clear(); right.clear();
	for(unsigned value : source) {
		if(value < subBlockLength) {
			left.insert(value);
		} else {
			right.insert(value-subBlockLength);
		}
	}
}


PolarCoder::PolarCoder(Encoding::Encoder *encoder, Decoding::Decoder *decoder)
	: mEncoder(encoder), mDecoder(decoder) {
}

}//namespace PolarCode
