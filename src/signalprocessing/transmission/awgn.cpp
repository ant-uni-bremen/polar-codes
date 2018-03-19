#include <signalprocessing/transmission/awgn.h>
#include <cmath>
#include <cstddef>
#include <random>
#include <immintrin.h>

namespace SignalProcessing {
namespace Transmission {

Awgn::Awgn() : Awgn(10.0) {}

Awgn::Awgn(float EsN0_dB) {
	//Create 256-bit pseudo-random generator
	mRandGen = new Random::Generator();

	setEsN0(EsN0_dB);
}

Awgn::~Awgn() {
	delete mRandGen;
}

void Awgn::setEsN0(float EsNo) {
	mEsNoLog = EsNo;
	mEsNoLin = pow(10.0, mEsNoLog/10.0);
	mNoiseMagnitude = 1.0/sqrt(mEsNoLin);
}

void Awgn::setEsN0Linear(float EsNo) {
	mEsNoLin = EsNo;
	mEsNoLog = 10.0*log10(EsNo);
	mNoiseMagnitude = 1.0/sqrt(mEsNoLin);
}

float Awgn::EsNo() {
	return mEsNoLog;
}

void Awgn::transmit() {
	size_t size = mSignal->size();

	if(size % 16 == 0) {
		return transmit_vectorized();
	} else {
		return transmit_simple();
	}
}

void Awgn::transmit_simple() {
	std::mt19937_64 generator;
	std::normal_distribution<float> distribution(0, mNoiseMagnitude);

	float *fSignal = mSignal->data();
	const size_t size = mSignal->size();

	for(size_t i=0; i<size; ++i) {
		fSignal[i] += distribution(generator);
	}

}

void Awgn::transmit_vectorized() {

	float *fSignal = mSignal->data();
	const size_t size = mSignal->size();

	__m256 noiseMagnitude = _mm256_set1_ps(mNoiseMagnitude);

	for(size_t i=0; i<size; i+=16) {
		//Generate Gaussian noise
		__m256 a, b;
		mRandGen->getNormDist(&a, &b);

		//Load signal
		__m256 siga = _mm256_loadu_ps(fSignal+i);
		__m256 sigb = _mm256_loadu_ps(fSignal+i+8);

		//Add noise to signal
#ifdef __FMA__
		siga = _mm256_fmadd_ps(noiseMagnitude, a, siga);
		sigb = _mm256_fmadd_ps(noiseMagnitude, b, sigb);
#else
		siga = _mm256_add_ps(_mm256_mul_ps(noiseMagnitude, a), siga);
		sigb = _mm256_add_ps(_mm256_mul_ps(noiseMagnitude, b), sigb);
#endif
		//Store signal
		_mm256_storeu_ps(fSignal+i,   siga);
		_mm256_storeu_ps(fSignal+i+8, sigb);

	}
}

}//namespace Transmission
}//namespace SignalProcessing

