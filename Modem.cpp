#include "Modem.h"

#include <cmath>
#include <cstdio>
#include <vector>

#include "ArrayFuncs.h"

using namespace std;

float one = 1.0;
unsigned int *iOne = reinterpret_cast<unsigned int*>(&one);

void modulate(vector<float> &signal, aligned_float_vector &data)
{
	int nBits = data.size();
	
	unsigned int *iSig = reinterpret_cast<unsigned int*>(signal.data());
	unsigned int *iData = reinterpret_cast<unsigned int*>(data.data());
	
	for(int i=0; i<nBits; ++i)
	{
		//signal[i] = data[i]? -1.0 : 1.0;
		iSig[i] = iData[i] | *iOne;
	}
}

void softDemod(vector<float> &LLR, vector<float> &signal, float R, float EbN0)
{	
	float EbN0lin = pow(10.0, EbN0/10.0);
	float factor = 2.0 * sqrt(2.0*R*EbN0lin);
	int length = signal.size();
	for(int i=0; i<length; ++i)
	{
		LLR[i] = signal[i]*factor;
	}
}

