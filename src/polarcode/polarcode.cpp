#include <polarcode/polarcode.h>
#include <vector>

namespace PolarCode {

void splitFrozenBits(const std::vector<unsigned> &source,
		size_t subBlockLength,
		std::vector<unsigned> &left,
		std::vector<unsigned> &right) {
	left.resize(source.size());
	right.resize(source.size());
	unsigned leftCounter = 0, rightCounter = 0;

	for(unsigned value : source) {
		if(value < subBlockLength) {
			left[leftCounter++] = value;
			//left.insert(value);
		} else {
			//right.insert(value-subBlockLength);
			right[rightCounter++] = value-subBlockLength;
		}
	}
	left.resize(leftCounter);
	right.resize(rightCounter);
}


PolarCoder::PolarCoder(Encoding::Encoder *encoder, Decoding::Decoder *decoder)
	: mEncoder(encoder), mDecoder(decoder) {
}

}//namespace PolarCode
