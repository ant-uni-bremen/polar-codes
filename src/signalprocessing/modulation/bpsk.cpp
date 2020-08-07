/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <signalprocessing/modulation/bpsk.h>

#include <immintrin.h>

namespace SignalProcessing {
namespace Modulation {

Bpsk::Bpsk() : Modem() {
}

Bpsk::~Bpsk() {
}

void Bpsk::modulate() {
	size_t size = mInputSignal->size();
	mOutputSignal->resize(size);

	if(size % 8 == 0) {
		return modulate_vectorized();
	} else {
		return modulate_simple();
	}
}

void Bpsk::demodulate() {
	size_t size = mInputSignal->size();
	mOutputSignal->resize(size);

	/* This is a hard-output demodulator.
	 * Let's not use this. In fact, the demodulation step reduces to a no-op,
	 * because the actually detected bit is already present in the sign bit of each
	 * symbol.
	 * When inserting the 'demodulated' signal into a BitContainer, soft-output
	 * precision might be lost (for PackedContainer and CharContainer).
	 *
		if(size % 8 == 0) {
			return demodulate_vectorized();
		} else {
			return demodulate_simple();
		}
	 */

	std::copy(mInputSignal->begin(), mInputSignal->end(), mOutputSignal->begin());
}

void Bpsk::modulate_simple() {
	static const float fAmplitude = 1.0;
	static const unsigned int *iAmplitude = reinterpret_cast<const unsigned int*>(&fAmplitude);
	unsigned int *iIn = reinterpret_cast<unsigned int*>(mInputSignal->data());
	unsigned int *iOut = reinterpret_cast<unsigned int*>(mOutputSignal->data());
	const unsigned size = mInputSignal->size();

	for(unsigned i=0; i<size; ++i) {
		iOut[i] = iIn[i] | *iAmplitude;
	}
}

void Bpsk::modulate_vectorized() {
	static const __m256 one = _mm256_set1_ps(1.0);
	const float* in = mInputSignal->data();
	float* out = mOutputSignal->data();
	const unsigned size = mInputSignal->size();

	for(unsigned i=0; i<size; i+=8) {
		__m256 symbol = _mm256_loadu_ps(in+i);
		symbol = _mm256_or_ps(symbol, one);
		_mm256_storeu_ps(out+i, symbol);
	}
}

/* unused */
void Bpsk::demodulate_simple() {
	static const unsigned int sgnMask = 0x80000000;
	unsigned int *iIn = reinterpret_cast<unsigned int*>(mInputSignal->data());
	unsigned int *iOut = reinterpret_cast<unsigned int*>(mOutputSignal->data());
	const unsigned size = mInputSignal->size();

	for(unsigned i=0; i<size; ++i) {
		iOut[i] = iIn[i] & sgnMask;
	}
}

/* unused */
void Bpsk::demodulate_vectorized() {
	static const __m256 sgnMask = _mm256_set1_ps(-0.0);
	const float* in = mInputSignal->data();
	float* out = mOutputSignal->data();
	const unsigned size = mInputSignal->size();

	for(unsigned i=0; i<size; i+=8) {
		__m256 symbol = _mm256_loadu_ps(in+i);
		symbol = _mm256_and_ps(symbol, sgnMask);
		_mm256_storeu_ps(out+i, symbol);
	}
}

}//namespace Modulation
}//namespace SignalProcessing
