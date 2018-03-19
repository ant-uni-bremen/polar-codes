#include <signalprocessing/transmission/rayleigh.h>
#include <cmath>
#include <cstddef>
#include <random>
#include <immintrin.h>

namespace SignalProcessing {
namespace Transmission {

Rayleigh::Rayleigh() : Rayleigh(10.0) {}

Rayleigh::Rayleigh(float EsN0_dB) {
	//Create 256-bit pseudo-random generator
	mRandGen = new Random::Generator();

	setEsN0(EsN0_dB);
}

Rayleigh::~Rayleigh() {
	delete mRandGen;
}

void Rayleigh::setEsN0(float EsNo) {
	mEsNoLog = EsNo;
	mEsNoLin = pow(10.0, mEsNoLog/10.0);
	mNoiseMagnitude = 1.0/sqrt(mEsNoLin);
}

void Rayleigh::setEsN0Linear(float EsNo) {
	mEsNoLin = EsNo;
	mEsNoLog = 10.0*log10(EsNo);
	mNoiseMagnitude = 1.0/sqrt(mEsNoLin);
}

float Rayleigh::EsNo() {
	return mEsNoLog;
}

void Rayleigh::transmit() {
	size_t size = mSignal->size();

	if(size % 16 == 0) {
		return transmit_vectorized();
	} else {
		return transmit_simple();
	}
}

void Rayleigh::transmit_simple() {
	std::mt19937_64 generator;
	std::normal_distribution<float> noiseDist(0.0, mNoiseMagnitude);
	std::normal_distribution<float> raylDist(0.0, 1.0);

	float *fSignal = mSignal->data();
	const size_t size = mSignal->size();
	float real, imag, rayleighFactor;

	for(size_t i=0; i<size; ++i) {
		real = raylDist(generator);
		imag = raylDist(generator);
		rayleighFactor = sqrt(real*real + imag*imag);

		fSignal[i] = fSignal[i] * rayleighFactor + noiseDist(generator);
	}
}

void Rayleigh::transmit_vectorized() {

	float *fSignal = mSignal->data();
	const size_t size = mSignal->size();

	__m256 noiseMagnitude = _mm256_set1_ps(mNoiseMagnitude);

	for(size_t i=0; i<size; i+=16) {
		//Generate Gaussian noise
		__m256 noiseA, noiseB;
		__m256 raylA, raylB;
		mRandGen->getNormDist(&noiseA, &noiseB);
		mRandGen->getRayleighDist(&raylA, &raylB);
		//mRandGen->getNormDist(&raylA, &raylB);

		//Load signal
		__m256 sigA = _mm256_loadu_ps(fSignal+i);
		__m256 sigB = _mm256_loadu_ps(fSignal+i+8);

		//Deform signal
		sigA = _mm256_mul_ps(sigA, raylA);
		sigB = _mm256_mul_ps(sigB, raylB);

		//Add noise to signal
#ifdef __FMA__
		sigA = _mm256_fmadd_ps(noiseMagnitude, noiseA, sigA);
		sigB = _mm256_fmadd_ps(noiseMagnitude, noiseB, sigB);
#else
		sigA = _mm256_add_ps(_mm256_mul_ps(noiseMagnitude, noiseA), sigA);
		sigB = _mm256_add_ps(_mm256_mul_ps(noiseMagnitude, noiseB), sigB);
#endif
		//Store signal
		_mm256_storeu_ps(fSignal+i,   sigA);
		_mm256_storeu_ps(fSignal+i+8, sigB);

	}
}

}//namespace Transmission
}//namespace SignalProcessing

