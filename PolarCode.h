#ifndef POLARCODE_H
#define POLARCODE_H

float logdomain_sum(float x, float y);
float logdomain_diff(float x, float y);
float upperconv(float a, float b);
float lowerconv(unsigned char bit, float upper, float lower);

struct PolarCode
{
	int N, K, L, n;
	unsigned char *FZLookup;
	float designSNR;
	int
		*bitreversed_indices,
		*index_of_first0_from_MSB,
		*index_of_first1_from_MSB;
		
	float **LLR;
	unsigned char **Bits;
	unsigned char **Dhat;
	float *Metric;
	int PathCount;
	float **NextLLR;
	unsigned char **NextBits;
	unsigned char **NextDhat;
	float *NextMetric;
	bool *NextPaths;
	bool memInitialized;
	
	PolarCode(int N, int K, int L, float designSNR);
	~PolarCode();
	
	/*
		K must be a multiple of 8
		data has to be one byte shorter than K/8, reserving
		space for the CRC8-checksum
	*/
	void encode(unsigned char *encoded, unsigned char *data);
	bool decode(unsigned char *decoded, float *LLR);
	
	
	
	
	
	unsigned int bitreversed_slow(unsigned int j);
	void pcc();
	void resetMemory();
	void updateLLR(int path, int i);
	void updateBits(int path, int d, int i);
	
};


#endif

