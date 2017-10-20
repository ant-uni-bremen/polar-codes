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
	mLcg = (Random::LCG<__m256>*)_mm_malloc(sizeof(Random::LCG<__m256>), 32);

	{//Seed it
		union {
			__m256i seed256;
			uint64_t seed64[4];
		};
		mRandGen->get64x4(seed64);
		mLcg->seed(seed256);
	}

	setEsN0(EsN0_dB);
}

Awgn::~Awgn() {
	delete mRandGen;
	delete mLcg;
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
	static const __m256 twopi = _mm256_set1_ps(2.0f * 3.14159265358979323846f);
	static const __m256 one = _mm256_set1_ps(1.0f);
	static const __m256 minustwo = _mm256_set1_ps(-2.0f);

	float *fSignal = mSignal->data();
	const size_t size = mSignal->size();

	__m256 noiseMagnitude = _mm256_set1_ps(mNoiseMagnitude);

	for(size_t i=0; i<size; i+=16) {
		//Generate Gaussian noise
		__m256 u1 = _mm256_sub_ps(one, (*mLcg)()); // [0, 1) -> (0, 1]
		__m256 u2 = (*mLcg)();
		__m256 radius = _mm256_mul_ps(
							noiseMagnitude,
							_mm256_sqrt_ps(_mm256_mul_ps(
												minustwo,
											   log256_ps(u1))));
		__m256 theta = _mm256_mul_ps(twopi, u2);
		__m256 sintheta, costheta;
		sincos256_ps(theta, &sintheta, &costheta);

		//Load signal
		__m256 siga = _mm256_loadu_ps(fSignal+i);
		__m256 sigb = _mm256_loadu_ps(fSignal+i+8);

		//Add noise to signal
#ifdef __FMA__
		siga = _mm256_fmadd_ps(radius, costheta, siga);
		sigb = _mm256_fmadd_ps(radius, sintheta, sigb);
#else
#warning FMA not used
		siga = _mm256_add_ps(_mm256_mul_ps(radius, costheta), siga);
		sigb = _mm256_add_ps(_mm256_mul_ps(radius, sintheta), sigb);
#endif
		//Store signal
		_mm256_storeu_ps(fSignal+i,   siga);
		_mm256_storeu_ps(fSignal+i+8, sigb);

	}
}

}//namespace Transmission
}//namespace SignalProcessing

