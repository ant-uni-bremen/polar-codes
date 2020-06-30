/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/construction/bhattacharrya.h>
#include <cmath>
#include <algorithm>

namespace PolarCode {
namespace Construction {

Bhattacharrya::Bhattacharrya()
	: mInitialParameter(0.5) {
}

Bhattacharrya::Bhattacharrya(size_t N, size_t K) {
	setBlockLength(N);
	setInformationLength(K);
	setParameterByDesignSNR(mDesignSnr);
}

Bhattacharrya::Bhattacharrya(size_t N, size_t K, float designSnr) {
	setBlockLength(N);
	setInformationLength(K);
	setParameterByDesignSNR(designSnr);
}

Bhattacharrya::~Bhattacharrya() {
}

void Bhattacharrya::setInitialParameter(float newInitialParameter) {
	mInitialParameter = newInitialParameter;
}

void Bhattacharrya::setParameterByDesignSNR(float designSNR) {
	float linearDesignSNR = pow(10.0, designSNR/10.0);
	mDesignSnr = designSNR;
	mInitialParameter = exp(-2.0 * linearDesignSNR
							* mInformationLength / mBlockLength);

}

std::vector<unsigned> Bhattacharrya::construct() {
	std::vector<unsigned> frozenBits;
	frozenBits.resize(mBlockLength - mInformationLength);

	//Generate and sort parameters
	calculateChannelParameters();
	mSorter.set(mChannelParameters);
	mSorter.stableSortDescending();

	//Select frozen channels
	for(unsigned channel=0;
			channel < (mBlockLength - mInformationLength);
			++channel) {
		frozenBits[channel] = mSorter.permuted[channel];
	}

	std::sort(frozenBits.begin(), frozenBits.end());

	return frozenBits;
}

void Bhattacharrya::calculateChannelParameters() {
	mChannelParameters.resize(mBlockLength);
	mChannelParameters[0] = mInitialParameter;

	double T; int B;
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
