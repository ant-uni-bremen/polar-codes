#include <polarcode/construction/bhattacharrya.h>
#include <cmath>

namespace PolarCode {
namespace Construction {

Bhattacharrya::Bhattacharrya()
	: mInitialParameter(0.5) {
}

Bhattacharrya::Bhattacharrya(size_t N, size_t K)
	: mInitialParameter(0.5) {
	setBlockLength(N);
	setInformationLength(K);
}

Bhattacharrya::Bhattacharrya(size_t N, size_t K, float param)
	: mInitialParameter(param) {
	setBlockLength(N);
	setInformationLength(K);
}

Bhattacharrya::~Bhattacharrya() {
}

void Bhattacharrya::setInitialParameter(float newInitialParameter) {
	mInitialParameter = newInitialParameter;
}

void Bhattacharrya::setParameterByDesignSNR(float designSNR) {
	float linearDesignSNR = pow(10.0, designSNR/10.0);
	mInitialParameter = exp(-2.0 * linearDesignSNR
							* mInformationLength / mBlockLength);

}

std::vector<unsigned> Bhattacharrya::construct() {
	std::vector<unsigned> frozenBits;
	frozenBits.resize(mBlockLength-mInformationLength);

	//Generate and sort parameters
	calculateChannelParameters();
	mSorter.set(mChannelParameters);
	mSorter.stableSortDescending();

	//Select frozen channels
	for(unsigned channel=0;
			channel < (mBlockLength-mInformationLength);
			++channel) {
		frozenBits[channel] = mSorter.permuted[channel];
	}

	return frozenBits;
}

void Bhattacharrya::calculateChannelParameters() {
	mChannelParameters.resize(mBlockLength);
	mChannelParameters[0] = mInitialParameter;

	float T; int B;
	for(signed stage=log2(mBlockLength)-1; stage >= 0; --stage) {
		B = 1<<stage;// = pow(2, stage);
		for(unsigned j = 0; j < mBlockLength; j+=(B<<1)) {
			T = mChannelParameters[j];
			mChannelParameters[j+B] = T*T;
			mChannelParameters[j] = 2*T - mChannelParameters[j+B];
		}
	}
}

}//namespace Construction
}//namespace PolarCode
