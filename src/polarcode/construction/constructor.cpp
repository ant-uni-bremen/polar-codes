#include <polarcode/construction/constructor.h>
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

}//namespace Construction
}//namespace PolarCode
