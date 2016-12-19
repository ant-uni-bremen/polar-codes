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
	RepSPCnode,
	RateR
};

struct Candidate
{
	int srcPath;//="currentPath"
	int decisionIndex;//=decision path index of respective constituent decoder
	float newMetric;
	int hints[4], nHints;
};

struct PolarCode
{
	float *AlignedVector;
	vec SIGN_MASK, ABS_MASK;
	int N, K, L, n;
	vector<int> FZLookup;
	float designSNR;
	
	vector<nodeInfo> simplifiedTree;
		
	typedef vector<float, aligned_allocator<float, sizeof(vec)> > aligned_float_vector;
		
	vector<vector<aligned_float_vector>> LLR;//[List][Stage][ValueIndex]
	vector<vector<vector<aligned_float_vector>>> Bits;//[List][Stage][LeftRight][ValueIndex]
	aligned_float_vector SimpleBits;
	vector<float> Metric;
	int PathCount;
	
	PolarCode(int N, int K, int L, float designSNR);
	~PolarCode();

	void encode(vector<bool> &encoded, vector<bool> &data);
	bool decode(vector<bool> &decoded, vector<float> &LLR);
	bool decodeOnePath(vector<bool> &decoded, vector<float> &LLR);
	bool decodeMultiPath(vector<bool> &decoded, vector<float> &LLR);
	
	void decodeOnePathRecursive(int stage, float *nodeBits, int nodeID);
	void decodeMultiPathRecursive(int stage, int BitLocation, int nodeID);
	void transform(aligned_float_vector &BitsIn, vector<bool> &BitsOut);
	
	void quick_abs(float *LLRin, float *LLRout, int size);
	
	void F_function(float *LLRin, float *LLRout, int size);
	void F_function_vectorized(float *LLRin, float *LLRout, int size);
	void F_function_hybrid(float *LLRin, float *LLRout, int size);
	
	void G_function(float *LLRin, float *LLRout, float *Bits, int size);
	void G_function_vectorized(float *LLRin, float *LLRout, float *Bits, int size);
	void G_function_hybrid(float *LLRin, float *LLRout, float *Bits, int size);
	
	void G_function_0R(float *LLRin, float *LLRout, int size);
	void G_function_0R_vectorized(float *LLRin, float *LLRout, int size);
	void G_function_0R_hybrid(float *LLRin, float *LLRout, int size);

	void Combine(float *BitsIn_l, float *BitsIn_r, float *BitsOut, int size);
	void Combine_0R(float *BitsIn_r, float *BitsOut, int size);
	
	void CombineSimple(float *Bits, int size);
	void Combine_0RSimple(float *Bits, int size);

	void SPC(float *LLRin, float *BitsOut, int size);
	void SPC_multiPath(int stage, int BitLocation);

	void Repetition(float *LLRin, float *BitsOut, int size);
	void Repetition_vectorized(float *LLRin, float *BitsOut, int size);
	void Repetition_hybrid(float *LLRin, float *BitsOut, int size);
	void Repetition_multiPath(int stage, int BitLocation);
	
	void RepSPC(float *LLRin, float *BitsOut, int size);
	void RepSPC_8(float *LLRin, float *BitsOut);
	
	void P_R1(float *LLRin, float *BitsOut, int size);
	void P_01(float *LLRin, float *BitsOut, int size);

	void Rate0(float *BitsOut, int size);
	void Rate0_multiPath(int stage, int BitLocation);
	void Rate1(float *LLRin, float *BitsOut, int size);
	void Rate1_multiPath(int stage, int BitLocation);


	unsigned int bitreversed_slow(unsigned int j);
	void pcc();
	void resetMemory();
	
};


#endif

