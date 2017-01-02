#ifndef POLARCODE_H
#define POLARCODE_H

#include <vector>

#include "Parameters.h"
#include "ArrayFuncs.h"
#include "AlignedAllocator.h"
#include "crc8.h"

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
	int hints[4], nHints;
};

struct PolarCode
{
	float *AlignedVector;
	vec SIGN_MASK, ABS_MASK, ZERO;
	__m128 sgnMask;
	__m128 absMask;

	int N, K, L, n;
	vector<int> FZLookup;
	vector<int> AcceleratedLookup;
	float designSNR;
	
	vector<nodeInfo> simplifiedTree;
		
	typedef vector<float, aligned_allocator<float, sizeof(vec)> > aligned_float_vector;
	
	aligned_float_vector initialLLR;
	vector<vector<aligned_float_vector>> LLR;//[List][Stage][ValueIndex]
	vector<aligned_float_vector> Bits;//[List]
	
	vector<vector<aligned_float_vector>> newLLR;//[List][Stage][ValueIndex]
	vector<aligned_float_vector> newBits;//[List]

	aligned_float_vector absLLR;
	trackingSorter sorter;
	
	aligned_float_vector SimpleBits;
	vector<float> Metric;
	
	vector<float> newMetrics;
	vector<Candidate> cand;
	
	int PathCount;
	int maxCandCount;
	CRC8 *Crc;
	
	vector<bool> SysX, SysY;
	
	PolarCode(int N, int K, int L, float designSNR, bool encodeOnly=false);
	~PolarCode();

	void encode(aligned_float_vector &encoded, float* data);
/*	void encode_systematic(vector<bool> &encoded, vector<bool> &data);
	
	void encoderB(int i, int j, vector<bool> &r, vector<bool> &v);*/
	
	bool decode(float* decoded, float* LLR);
	bool decodeOnePath(float* decoded);
	bool decodeMultiPath(float* ndecoded);
	
	void decodeOnePathRecursive(int stage, float *nodeBits, int nodeID);
	void decodeMultiPathRecursive(int stage, int BitLocation, int nodeID);
	void transform(aligned_float_vector &Bits);
	
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

	void CombineSimple(float *Bits, int size);
	void CombineSimple_vectorized(float *Bits, int size);
	void Combine_0RSimple(float *Bits, int size);

	void SPC(float *LLRin, float *BitsOut, int size);
	void SPC_4(float *LLRin, float *BitsOut);
	void SPC_multiPath(int stage, int BitLocation);
	
	void P_RSPC(float *LLRin, float *BitsOut, int size);
	void P_0SPC(float *LLRin, float *BitsOut, int size);
	void P_0SPC_vectorized(float *LLRin, float *BitsOut, int size);

	void Repetition(float *LLRin, float *BitsOut, int size);
	void Repetition_vectorized(float *LLRin, float *BitsOut, int size);
	void Repetition_hybrid(float *LLRin, float *BitsOut, int size);
	void Repetition_multiPath(int stage, int BitLocation);
	
	void RepSPC(float *LLRin, float *BitsOut, int size);
	void RepSPC_8(float *LLRin, float *BitsOut);
	
	void P_R1(float *LLRin, float *BitsOut, int size);
	void P_R1_vectorized(float *LLRin, float *BitsOut, int size);
	void P_01(float *LLRin, float *BitsOut, int size);

	void Rate0(float *BitsOut, int size);
	void Rate0_multiPath(int stage, int BitLocation);
	void Rate1(float *LLRin, float *BitsOut, int size);
	void Rate1_vectorized(float *LLRin, float *BitsOut, int size);
	void Rate1_multiPath(int stage, int BitLocation);


	unsigned int bitreversed_slow(unsigned int j);
	void pcc();
	void clear();
	
};


#endif

