#include <atomic>
#include <chrono>
#include <cmath>
#include <complex>
#include <condition_variable>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

#include "ArrayFuncs.h"
#include "PolarCode.h"
#include "Modem.h"

#include "Parameters.h"


std::atomic<int> nextThread(-1), finishedThreads(0);
//std::map<int, std::atomic<float>> stopSNR;

struct DataPoint
{
	float designSNR, EbN0; int N,K,L;
	int runs, errors;
	float BLER, BER;
	float time;//in seconds
	float blps,cbps,pbps;//blocks/coded bits/payload bits per second
	float effectiveRate;
} *Graph;

void simulate(int SimIndex)
{
	using namespace std;
	using namespace std::chrono;
	
	int L = Graph[SimIndex].L;
	float designSNR = Graph[SimIndex].designSNR;//dB
	float EbN0 = Graph[SimIndex].EbN0;//dB
	
	float R = (float)Graph[SimIndex].K / Graph[SimIndex].N;
	
	int runs = 0, errors = 0, biterrors = 0;
	char message[128];
	
	while(SimIndex > nextThread/* && EbN0 < stopSNR[L]*/)
	{
//		std::this_thread::yield();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	
/*	if(EbN0 >= stopSNR[L])
	{*/
		/* This might save some time.
		   Best case: Lowest SNR which reaches MaxIter iterations
		              is simulated first.
		   Worst case: Simulations run from highest to lowest SNRs
					   with BLER=0.
		   Running all SNRs squentially might leave some processor
		   cores unused near the end of simulation.
		   Especially for long lists, it is better to run multiple
		   SNRs per same list size in parallel. Otherwise, if size
		   1 has finished, there will be a core left unused, while
		   size 8 simulations waits a long time.
		*/
/*		sprintf(message, "[%3d] Skipping Eb/N0 = %f dB, L = %d\n", SimIndex, EbN0, L);
		std::cout << message;
		Graph[SimIndex].runs = 0;
		Graph[SimIndex].errors = 0;
		Graph[SimIndex].BLER = 0;
		Graph[SimIndex].time = 0;
		Graph[SimIndex].blps = 0;
		Graph[SimIndex].cbps = 0;
		Graph[SimIndex].pbps = 0;
		Graph[SimIndex].effectiveRate = 0;
		++finishedThreads;
		
		return;
	}*/
	
//	++runningThreads;
	
	sprintf(message, "[%3d] Generating samples for Eb/N0 = %f dB\n", SimIndex, EbN0);
	std::cout << message;

	//Encoder
	PolarCode PC(Graph[SimIndex].N, Graph[SimIndex].K, L, designSNR);

#ifdef CRCSIZE
	int nBits = (Graph[SimIndex].K-CRCSIZE);
#else
	int nBits = Graph[SimIndex].K;
#endif

	default_random_engine RndGen(1);
	uniform_int_distribution<unsigned char> RndDist(0, 1);
	normal_distribution<float> NormDist(0.0, 1.0);
	vector<vector<bool>> data(MaxIters, vector<bool>(nBits,false));
	vector<vector<bool>> decodedData(MaxIters, vector<bool>(Graph[SimIndex].K,false));
	vector<bool> decodingSuccess(MaxIters, false);
	vector<bool> encodedData(Graph[SimIndex].N, false);
	
	vector<float> sig(Graph[SimIndex].N,0.0);
	vector<vector<float>> LLR(MaxIters, vector<float>(Graph[SimIndex].N, 0.0));
	//vector<bool> decodedData(PCparam_N,false);
	
	for(int block = 0; block<MaxIters; ++block)
	{
		//Generate random payload for testing
		for(int i=0; i<nBits; ++i)
		{
			data[block][i] = RndDist(RndGen);
		}
			
		//Encode
		PC.encode(encodedData, data[block]);
			
		//Modulate using simple BPSK
		modulate(sig, encodedData);
		
		//Distort / Transmit via AWGN-channel
		//(Signal+Noise) is normalized to noise power of 1
		// afterwards it is equalized for some perfectly known rayleigh fading channel
		float factor = sqrt(R) * pow(10.0, EbN0/20.0)  * sqrt(2.0);
//		std::complex<float> channelCoeff, noise, sample;
		
		for(int i=0; i<Graph[SimIndex].N; ++i)
		{
/*			sample.real(sig[block][i]*factor);
			sample.imag(0.0);
			channelCoeff.real(NormDist(RndGen));
			channelCoeff.imag(NormDist(RndGen));
			noise.real(NormDist(RndGen));
			noise.imag(NormDist(RndGen));
			
			noise /= channelCoeff;
			sample += noise;
			sig[i] = sample.real();*/
			sig[i] = sig[i]*factor + NormDist(RndGen);
		}
			
		//Demodulate
		softDemod(LLR[block], sig, R, EbN0);
	}

	sprintf(message, "[%3d] Decoding with L = %d\n", SimIndex, L);
	std::cout << message;

	high_resolution_clock::time_point TimeStart = high_resolution_clock::now();
	
	while(runs < MaxIters/* && !(errors>=MinErrors && runs>=MinIters)*/)
	{
		//Decode
		decodingSuccess[runs] = PC.decode(decodedData[runs], LLR[runs]);
/*		if(!decodingSuccess[runs])
		{
			++errors;
		}*/
		++runs;
	}
	
	high_resolution_clock::time_point TimeEnd = high_resolution_clock::now();
	duration<float> TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart);
	float time = TimeUsed.count();

	sprintf(message, "[%3d] Counting BER\n", SimIndex);
	std::cout << message;

	errors = 0;
	
	for(int block=0; block<runs; ++block)
	{
		bool errorFound = false;
		for(int i=0; i<nBits; ++i)
		{
			if(decodedData[block][i] != data[block][i])
			{
				if(!errorFound)
				{
					++errors;
					errorFound = true;
				}
				++biterrors;
			}
		}
		decodedData[block].clear();
	}

	Graph[SimIndex].runs = runs;
	Graph[SimIndex].errors = errors;
	Graph[SimIndex].BLER = errors;
	Graph[SimIndex].BLER /= runs;
	Graph[SimIndex].BER = biterrors;
	Graph[SimIndex].BER /= runs*nBits;
	Graph[SimIndex].time = time;
	Graph[SimIndex].blps = runs;
	Graph[SimIndex].cbps = runs*Graph[SimIndex].N;
	Graph[SimIndex].pbps = runs*nBits;
	Graph[SimIndex].blps /= time;
	Graph[SimIndex].cbps /= time;
	Graph[SimIndex].pbps /= time;
	Graph[SimIndex].effectiveRate = (runs-errors+0.0)*nBits/time;
	
	
	
/*	if(runs == MaxIters && EbN0 < stopSNR[L])
	{
		stopSNR[L] = EbN0;
	}*/
	
	int finished = ++finishedThreads;
	sprintf(message, "[%3d] %3d Threads finished, BLER = %e\n", SimIndex, finished, Graph[SimIndex].BLER);
	std::cout << message;
	
//	--runningThreads;
	++nextThread;
}


int main(int argc, char** argv)
{
//	int Parameter[] = {1}, nParams = 1;
	int Parameter[] = {1, 2, 4, 8, 16}, nParams = 5;
//	float Parameter[] = {0, 2, 5, 6, 10}; int nParams = 5;


	Graph = new DataPoint[EbN0_count*nParams];
	std::vector<std::thread> Threads;
	
	std::ofstream File("../results/Simulation.csv");
	if(!File.is_open())
	{
		std::cout << "Error opening the file!" << std::endl;
		return 0;
	}

#ifdef __DEBUG__
	nextThread = 0;
#endif

	int idCounter = 0;
	for(int l=0; l<nParams; ++l)
	{
//		stopSNR[designSNRs[l]] = INFINITY;
		for(int i=0; i<EbN0_count; ++i)
		{
			Graph[idCounter].EbN0 = EbN0_min + (EbN0_max-EbN0_min)/(EbN0_count-1)*i;
			Graph[idCounter].N = 128;
			Graph[idCounter].K = 72;
			Graph[idCounter].L = Parameter[l];
			Graph[idCounter].designSNR = 5.0;
			//Graph[idCounter].L = 1;
			//Graph[idCounter].designSNR = Parameter[l];
			
#ifndef __DEBUG__
			Threads.push_back(std::thread(simulate, idCounter++));
#else
			simulate(idCounter++);
#endif
		}
	}

#ifndef __DEBUG__
	nextThread = ConcurrentThreads-1;
#endif

#ifndef __DEBUG__
	for(auto& Thr : Threads)
	{
		Thr.join();
	}
#endif
	
	File << "\"L\", \"Eb/N0\", \"BLER\", \"BER\", \"Runs\", \"Errors\",\"Time\",\"Blockspeed\",\"Coded Bitrate\",\"Payload Bitrate\",\"Effective Payload Bitrate\"" << std::endl;

	for(int i=0; i<EbN0_count*nParams; ++i)
	{
		File << Graph[i].L << ',' << Graph[i].EbN0 << ',';
		if(Graph[i].BLER>0.0)
		{
			File << Graph[i].BLER << ',';
		}
		else
		{
			File << "\" \",";
		}
		if(Graph[i].BER>0.0)
		{
			File << Graph[i].BER << ',';
		}
		else
		{
			File << "\" \",";
		}
		File << Graph[i].runs << ',' << Graph[i].errors << ','
		     << Graph[i].time << ',' << Graph[i].blps << ','
		     << Graph[i].cbps << ',' << Graph[i].pbps << ',';
		if(Graph[i].effectiveRate != 0)
		{
			File << Graph[i].effectiveRate;
		}
		else
		{
			File << "\" \"";
		}
		File << std::endl;
	}
	
	File.close();
	
	return 0;
}
