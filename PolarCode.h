#ifndef POLARCODE_H
#define POLARCODE_H

#include <vector>

#include "Parameters.h"
#include "ArrayFuncs.h"
#include "AlignedAllocator.h"
#include "crc8.h"
#include "DataPool.h"

#ifdef __AVX2__
#define USE_AVX2
#else
#define USE_AVX
#endif
#include "lcg.h"

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
	int hints[4], nHints;
};

struct PolarCode
{
	int N, K, L, n;
	bool useCRC, systematic;
	float designSNR;
	bool hasDecoder;
	
	vector<int> FZLookup;
	vector<int> AcceleratedLookup, AcceleratedFrozenLookup;

	vector<nodeInfo> simplifiedTree;

	float* initialLLR;
	vector<float> Metric;
	vector<vector<Block*>> LlrTree;//[List][Stage][ValueIndex]
	vector<Block*> Bits;//[List]
	
	vector<float> newMetrics;
	vector<vector<Block*>> newLlrTree;//[List][Stage][ValueIndex]
	vector<Block*> newBits;//[List]
	unsigned char **decodedData;

	aligned_float_vector absLLR;
	trackingSorter *sorter;
	
	vector<aligned_float_vector> simpleLlr;
	aligned_float_vector SimpleBits;
	vector<Candidate> cand;
	
	int PathCount;
	int maxCandCount;
	CRC8 *Crc;
	unsigned int **crcBit;
	unsigned char *crcByte;
	
	DataPool *pool;

	PolarCode(int N, int K, int L, bool useCRC, float designSNR, bool systematic, bool encodeOnly=false);
	~PolarCode();

	void encode(aligned_float_vector &encoded, unsigned char* data);
	void subEncodeSystematic(aligned_float_vector &encoded, int stage, int BitLocation, int nodeID);
		
	bool decode(unsigned char* decoded, float* LLR);
	bool decodeOnePath(unsigned char* decoded);
	bool decodeMultiPath(unsigned char* decoded);
	void cleanupBits();
	
	void decodeOnePathRecursive(int stage, float *nodeBits, int nodeID);
	void decodeMultiPathRecursive(int stage, int BitLocation, int nodeID);
	void transform(float *Bits);
	
	void quick_abs(float *LLRin, float *LLRout, int size);
	
	void F_function(float *LLRin, float *LLRout, int size);
	void F_function_vectorized(float *LLRin, float *LLRout, int size);
	void F_function_vectorized_4(float *LLRin, float *LLRout);

	void G_function(float *LLRin, float *LLRout, float *Bits, int size);
	void G_function_vectorized(float *LLRin, float *LLRout, float *Bits, int size);
	void G_function_vectorized_4(float *LLRin, float *LLRout, float *Bits);
	
	void G_function_0R(float *LLRin, float *LLRout, int size);
	void G_function_0R_vectorized(float *LLRin, float *LLRout, int size);
	void G_function_0R_vectorized_4(float *LLRin, float *LLRout);

	void CombineSimple(float *Bits, int size);
	void CombineSimple_vectorized(float *Bits, int size);
	void CombineSimple_vectorized_4(float *Bits);
	void Combine_0RSimple(float *Bits, int size);

	void SPC(float *LLRin, float *BitsOut, int size);
	void SPC_vectorized(float *LLRin, float *BitsOut, int size);
	void SPC_vectorized_4(float *LLRin, float *BitsOut);
	void SPC_multiPath(int stage, int BitLocation);
	
	void P_RSPC(float *LLRin, float *BitsOut, int size);
	void P_RSPC_4(float *LLRin, float *BitsOut);
	void P_0SPC(float *LLRin, float *BitsOut, int size);
	void P_0SPC_vectorized(float *LLRin, float *BitsOut, int size);
	void P_0SPC_vectorized_4(float *LLRin, float *BitsOut);

	void Repetition(float *LLRin, float *BitsOut, int size);
	void Repetition_vectorized(float *LLRin, float *BitsOut, int size);
	void Repetition_vectorized_4(float *LLRin, float *BitsOut);
	void Repetition_multiPath(int stage, int BitLocation);
	
//	void RepSPC(float *LLRin, float *BitsOut, int size);
	void RepSPC_8(float *LLRin, float *BitsOut);
	
	void P_R1(float *LLRin, float *BitsOut, int size);
	void P_R1_vectorized(float *LLRin, float *BitsOut, int size);
	void P_R1_vectorized_4(float *LLRin, float *BitsOut);
	void P_01(float *LLRin, float *BitsOut, int size);

	void Rate0(float *BitsOut, int size);
	void Rate0_multiPath(int stage, int BitLocation);
	void Rate1(float *LLRin, float *BitsOut, int size);
	void Rate1_vectorized(float *LLRin, float *BitsOut, int size);
	void Rate1_vectorized_4(float *LLRin, float *BitsOut);
	void Rate1_multiPath(int stage, int BitLocation);

	void pcc();
	
};


#endif

