#include <polarcode/construction/constructor.h>
#include <polarcode/construction/bhattacharrya.h>
#include <cmath>

namespace PolarCode {
namespace Construction {

Constructor::Constructor()
	: mBlockLength(0), mInformationLength(0) {
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

void Constructor::setParameterByDesignSNR(float designSNR){
  // FIXME: This is a dummy!
}


std::vector<unsigned> frozen_bits(const int blockLength, const int infoLength, const float designSNR) {
	PolarCode::Construction::Constructor *constructor
					= new PolarCode::Construction::Bhattacharrya(blockLength, infoLength);
	constructor->setParameterByDesignSNR(designSNR);
	return constructor->construct();
}
}//namespace Construction
}//namespace PolarCode
