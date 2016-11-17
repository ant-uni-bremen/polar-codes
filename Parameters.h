#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "SpecialParameters.h"
#define CRCSIZE 8

#define EbN0_min    0.0
#define EbN0_max    8.0
#define EbN0_count 10

#define ConcurrentThreads 2

#define MinErrors     10000
#define MinIters      50000
#define MaxIters    1000000

#ifdef __DEBUG__
//Can be commented out to suppress debugging output:
#define RATEONEDEBUGOUTPUT
#define DEBUGOUTPUT
#endif



/*
	IMPORTANT: Remember to recompile every(!) file that relies on this header!!!
*/






/*
	AVX512: 512 bit per register
	AVX:    256 bit per register
	float:   32 bit per value
*/

//#define CONFIG_AVX512

#ifdef CONFIG_AVX512

#include <immintrin.h>

#define FLOATSPERVECTOR 16
#define vec __m512

#define set1_ps _mm512_set1_ps
#define load_ps _mm512_load_ps
#define store_ps _mm512_store_ps

#define and_ps _mm512_and_ps
#define andnot_ps _mm512_andnot_ps
#define or_ps _mm512_or_ps
#define xor_ps _mm512_xor_ps

#define add_ps _mm512_add_ps
#define mul_ps _mm512_mul_ps

#define min_ps _mm512_min_ps

#else

#include <immintrin.h>

#define FLOATSPERVECTOR 8
#define vec __m256

#define set1_ps _mm256_set1_ps
#define load_ps _mm256_load_ps
#define store_ps _mm256_store_ps

#define and_ps _mm256_and_ps
#define andnot_ps _mm256_andnot_ps
#define or_ps _mm256_or_ps
#define xor_ps _mm256_xor_ps

#define add_ps _mm256_add_ps
#define mul_ps _mm256_mul_ps

#define min_ps _mm256_min_ps

#endif
#endif
