#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

#include "Modem.h"
#include "ArrayFuncs.h"

#ifdef __AVX2__
#define USE_AVX2
#else
#define USE_AVX
#endif
#include "lcg.h"

//#include <immintrin.h>

using namespace std;

/*
static void normaldistf_boxmuller_avx(float* data, size_t count) {
	assert(count % 16 == 0);
	const __m256 twopi = _mm256_set1_ps(2.0f * 3.14159265358979323846f);
	const __m256 one = _mm256_set1_ps(1.0f);
	const __m256 minustwo = _mm256_set1_ps(-2.0f);

	LCG<__m256> r;
	for (size_t i = 0; i < count; i += 16) {
        __m256 u1 = _mm256_sub_ps(one, r()); // [0, 1) -> (0, 1]
        __m256 u2 = r();
		__m256 radius = _mm256_sqrt_ps(_mm256_mul_ps(minustwo, log256_ps(u1)));
		__m256 theta = _mm256_mul_ps(twopi, u2);
        __m256 sintheta, costheta;
        sincos256_ps(theta, &sintheta, &costheta);
		_mm256_store_ps(&data[i    ], _mm256_mul_ps(radius, costheta));
		_mm256_store_ps(&data[i + 8], _mm256_mul_ps(radius, sintheta));
	}
}*/


const __m256 twopi = _mm256_set1_ps(2.0f * 3.14159265358979323846f);
const __m256 one = _mm256_set1_ps(1.0f);
const __m256 minustwo = _mm256_set1_ps(-2.0f);
LCG<__m256> r;

void modulateAndDistort(aligned_float_vector &signal, aligned_float_vector &data, int size, float factor)
{
	assert(size % 16 == 0);

	vec facVec = set1_ps(factor);
	for(int i=0; i<size; i+=16)
	{
		vec siga = load_ps(data.data()+i);
		vec sigb = load_ps(data.data()+i+8);
		siga = or_ps(siga, one); sigb = or_ps(sigb, one);//Modulate
		siga = mul_ps(siga, facVec); sigb = mul_ps(sigb, facVec);//Scale
		
        __m256 u1 = _mm256_sub_ps(one, r()); // [0, 1) -> (0, 1]
        __m256 u2 = r();
		__m256 radius = _mm256_sqrt_ps(_mm256_mul_ps(minustwo, log256_ps(u1)));
		__m256 theta = _mm256_mul_ps(twopi, u2);
        __m256 sintheta, costheta;
        sincos256_ps(theta, &sintheta, &costheta);

#ifdef __FMA__
		siga = _mm256_fmadd_ps(radius, costheta, siga);
		sigb = _mm256_fmadd_ps(radius, sintheta, sigb);
#else
		siga = add_ps(mul_ps(radius, costheta), siga);
		sigb = add_ps(mul_ps(radius, sintheta), sigb);
#endif
		
		store_ps(signal.data()+i, siga);
		store_ps(signal.data()+i+8, sigb);
	}
}

void softDemod(float *LLR, aligned_float_vector &signal, int size, float R, float EbN0)
{	
	float EbN0lin = pow(10.0, EbN0*0.1);
	float factor = 2.0 * sqrt(2.0*R*EbN0lin);
	vec facVec = set1_ps(factor);

	for(int i=0; i<size; i+=FLOATSPERVECTOR)
	{
		vec llr = load_ps(signal.data()+i);
		llr = mul_ps(llr, facVec);
		store_ps(LLR+i, llr);
	}
}

