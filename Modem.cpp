#include "Modem.h"

#include <cmath>
#include <cstdio>
#include <vector>

#include "ArrayFuncs.h"

using namespace std;

void modulate(vector<float> &signal, vector<bool> &data)
{
	int nBits = data.size();
	
	for(int i=0; i<nBits; ++i)
	{
		signal[i] = data[i]? -1.0 : 1.0;
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

