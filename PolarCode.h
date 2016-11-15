#ifndef POLARCODE_H
#define POLARCODE_H

#include <vector>

#include "Parameters.h"
#include "AlignedAllocator.h"

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

#define min_ps _mm256_min_ps

#endif


float logdomain_sum(float x, float y);
float logdomain_diff(float x, float y);

using namespace std;

enum nodeInfo
{
	RateZero,
	RateOne,
	RateHalf,
	RepetitionNode,
	SPCnode,
	RateR
};

struct PolarCode
{
	float *AlignedVector;
	vec SIGN_MASK;
	int L, n;
	vector<int> FZLookup;
	float designSNR;
	
	vector<nodeInfo> simplifiedTree;
		
	typedef vector<float, aligned_allocator<float, sizeof(vec)> > aligned_float_vector;
		
	vector<vector<aligned_float_vector>> LLR;//[List][Stage][ValueIndex]
	vector<vector<vector<aligned_float_vector>>> Bits;//[List][Stage][LeftRight][ValueIndex]
	vector<float> Metric;
	int PathCount;
	vector<float> NextMetric;
	vector<bool> NextPaths;
	
	PolarCode(int L, float designSNR);
	~PolarCode();
	
	/*
		K must be a multiple of 8
		data has to be eight bits shorter than K, reserving
		space for the CRC8-checksum
	*/
	void encode(vector<bool> &encoded, vector<bool> &data);
	bool decode(vector<bool> &decoded, vector<float> &LLR);
	bool decodeOnePath(vector<bool> &decoded, vector<float> &LLR);
	bool decodeMultiPath(vector<bool> &decoded, vector<float> &LLR);
	
	void decodeOnePathRecursive(int stage, int BitLocation, int nodeID);
	void transform(aligned_float_vector &BitsIn, vector<bool> &BitsOut);
	
	void F_function(float *LLRin, float *LLRout, int size);
	void G_function(float *LLRin, float *LLRout, float *Bits, int size);
	void Combine(float *BitsIn_l, float *BitsIn_r, float *BitsOut, int size);
	void SPC(float *LLRin, float *BitsOut, int size);
	void Repetition(float *LLRin, float *BitsOut, int size);
	void Rate0(float *BitsOut, int size);
	void Rate1(float *LLRin, float *BitsOut, int size);
	
	
	
	unsigned int bitreversed_slow(unsigned int j);
	void pcc();
	void resetMemory();
	
};


#endif

