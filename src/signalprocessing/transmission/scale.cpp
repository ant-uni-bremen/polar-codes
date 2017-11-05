#include <signalprocessing/transmission/scale.h>
#include <cmath>
#include <cstddef>
#include <random>
#include <immintrin.h>

namespace SignalProcessing {
namespace Transmission {

Scale::Scale() : Scale(1.0) {}

Scale::Scale(float factor)
	: mFactor(factor)
	{
}

Scale::~Scale() {
}

void Scale::setFactor(float factor) {
	mFactor = factor;
}

float Scale::Factor() {
	return mFactor;
}

void Scale::transmit() {
	size_t size = mSignal->size();

	if(size % 16 == 0) {
		return transmit_vectorized();
	} else {
		return transmit_simple();
	}
}

void Scale::transmit_simple() {
	float *fSignal = mSignal->data();
	const size_t size = mSignal->size();

	for(size_t i=0; i<size; ++i) {
		fSignal[i] *= mFactor;
	}

}

void Scale::transmit_vectorized() {
	float *fSignal = mSignal->data();
	const size_t size = mSignal->size();
	const __m256 factor = _mm256_set1_ps(mFactor);

	for(size_t i=0; i<size; i+=8) {
		__m256 signal = _mm256_loadu_ps(fSignal+i);
		signal = _mm256_mul_ps(signal, factor);
		_mm256_storeu_ps(fSignal+i, signal);
	}
}

}//namespace Transmission
}//namespace SignalProcessing

