#include <polarcode/construction/constructor.h>
#include <polarcode/construction/bhattacharrya.h>
#include <cmath>

namespace PolarCode {
namespace Construction {

Constructor::Constructor()
	: mBlockLength(0)
	, mInformationLength(0)
	, mDesignSnr(0.0) {
}

Constructor::~Constructor() {
}

void Constructor::setBlockLength(size_t newBlockLength)
		throw (InvalidBlockLengthException) {
	size_t testLength = 1 << static_cast<size_t>(log2(newBlockLength));
	if(testLength != newBlockLength) {
		throw InvalidBlockLengthException();
	}
	mBlockLength = newBlockLength;
}

void Constructor::setInformationLength(size_t newInformationLength) {
	mInformationLength = newInformationLength;
}

void Constructor::setDesignSnr(float designSnr){
	mDesignSnr = designSnr;
}

std::vector<unsigned> frozen_bits(const int blockLength, const int infoLength, const float designSnr) {
	auto constructor = new PolarCode::Construction::Bhattacharrya(blockLength, infoLength, designSnr);
	auto indices = constructor->construct();
	delete constructor;
	return indices;
}

}//namespace Construction
}//namespace PolarCode
