#include <iostream>
#include <random>
#include <cmath>
#include <cstring>

#include "PolarCode.h"
#include "Modem.h"

#include "ArrayFuncs.h"

int main(int argc, char** argv) {
	
	int N = 1<<7, K = (1<<6)+8, L = 1;
	float designSNR = 0.0;//dB
	float EbN0 = 2.0;//dB
	
	float R = (float)K/N;
	
	//Encoder
	PolarCode PC(N, K, L, designSNR);
		
	//Generate random payload for testing
	std::default_random_engine RndGen;
	std::uniform_int_distribution<unsigned char> RndDist(0, 255);
	
	int nBytes = (K-8)>>3;
	unsigned char *data = new unsigned char[nBytes];
	for(int i=0; i<nBytes; ++i)
	{
		data[i] = RndDist(RndGen);
	} 
		
	//Encode
	unsigned char *encodedData = new unsigned char[N>>3];
	PC.encode(encodedData, data);
		
	//Modulate using simple BPSK
	float *signal = new float[N];
	modulate(signal, encodedData, N>>3);
	
	//Distort / Transmit via normalized AWGN-channel
	std::normal_distribution<float> NormDist(0.0, 1.0);
	float factor = sqrt(R) * pow(10.0, EbN0/20.0)  * sqrt(2.0);
	float *sigptr = signal;
	
	for(int i=0; i<N; ++i)
	{
		*sigptr *= factor;
		*sigptr += NormDist(RndGen);
		++sigptr;
	}
		
	//Demodulate
	float *LLR = new float[N];
	softDemod(LLR, signal, N, R, EbN0);
	
	
	//Decode
	unsigned char *decodedData = new unsigned char[nBytes];
	if(PC.decode(decodedData, LLR))
	{
		puts("Reported success!");
	}
	else
	{
		puts("Reported failure!");
	}
	
	if(!memcmp(data, decodedData, nBytes))
	{
		puts("Success!");
	}
	else
	{
		puts("Failure!");
	}
	
	delete[] decodedData;
	delete[] LLR;
	delete[] encodedData;
	delete[] data;
	
	return 0;
}
