#ifndef POLARCODE_H
#define POLARCODE_H

#include "Parameters.h"

#include <vector>

float logdomain_sum(float x, float y);
float logdomain_diff(float x, float y);
float upperconv(float a, float b);
float lowerconv(unsigned char bit, float upper, float lower);


using namespace std;

struct PolarCode
{
	int L, n;
	vector<int> FZLookup;
	float designSNR;
	vector<int>
		bitreversed_indices,
		index_of_first0_from_MSB,
		index_of_first1_from_MSB;
		
	vector<vector<float>> LLR;
	vector<vector<bool>> Bits;
	vector<vector<bool>> Dhat;
	vector<float> Metric;
	int PathCount;
	vector<vector<float>>  NextLLR;
	vector<vector<bool>> NextBits;
	vector<vector<bool>> NextDhat;
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
	
	
	
	unsigned int bitreversed_slow(unsigned int j);
	void pcc();
	void resetMemory();
	void updateLLR(int path, int i);
	void updateBits(int path, int d, int i);
	
};


#endif

