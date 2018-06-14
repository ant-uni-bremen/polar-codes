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


PolarCoder::PolarCoder()
	: mBlockLength(0)
	, mInformationLength(0)
	, mSystematic(true)
	, mDesignSnr(0.0f)
	, mFrozenBits({})
	, mEncoder(new Encoding::UndefinedEncoder())
	, mDecoder(new Decoding::UndefinedDecoder())
{
}

PolarCoder::~PolarCoder() {
	delete mEncoder;
	delete mDecoder;
}

void PolarCoder::setCode(unsigned N, unsigned K, bool systematic) {
	mBlockLength = N;
	mInformationLength = K;
	mSystematic = systematic;
}

void PolarCoder::setDesignSnr(float dSnr) {
	mDesignSnr = dSnr;
}



}//namespace PolarCode
