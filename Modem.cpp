#include "Modem.h"

#include <cmath>

#include <cstdio>

#include "ArrayFuncs.h"
/*
Modem::Modem()
{
}

Modem::~Modem()
{
}*/
void modulate(float *signal, unsigned char *data, int nBytes)
{
	int nBits = nBytes<<3;
	unsigned char *bits = new unsigned char[nBits];
	Bytes2Bits(data, bits, nBytes);
	
	unsigned char *bitptr = bits;
	for(int i=0; i<nBits; ++i)
	{
		*signal = *bitptr? -1.0 : 1.0;
		signal++; bitptr++;
	}
	delete[] bits;
}

void softDemod(float *LLR, float *signal, int length, float R, float EbN0)
{
	if(length<=0) return;
	
	float EbN0lin = pow(10.0, EbN0/10.0);
	float factor = 2.0 * sqrt(2.0*R*EbN0lin);
	for(int i=0; i<length; ++i)
	{
		*LLR = (*signal)*factor;
		LLR++; signal++;
	}
}

