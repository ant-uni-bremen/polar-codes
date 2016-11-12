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


std::atomic<int> runningThreads(0), finishedThreads(0);
std::map<int, std::atomic<float>> stopSNR;

struct DataPoint
{
	float EbN0; int L;
	int runs, errors;
	float BLER;
	float time;//in seconds
	float blps,cbps,pbps;//blocks/coded bits/payload bits per second
	float effectiveRate;
} *Graph;

void simulate(int SimIndex)
{
	using namespace std;
	using namespace std::chrono;
	
	int L = Graph[SimIndex].L;
	float designSNR = 0.0;//dB
	float EbN0 = Graph[SimIndex].EbN0;//dB
	
	float R = (float)PCparam_K/PCparam_N;
	
	int runs = 0, errors = 0;
	char message[128];
	
	while(runningThreads >= ConcurrentThreads && EbN0 < stopSNR[L])
	{
//		std::this_thread::yield();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	if(EbN0 >= stopSNR[L])
	{
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
		sprintf(message, "[%3d] Skipping Eb/N0 = %f dB, L = %d\n", SimIndex, EbN0, L);
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
	}
	
	++runningThreads;
	
	sprintf(message, "[%3d] Generating samples for Eb/N0 = %f dB\n", SimIndex, EbN0);
	std::cout << message;

	//Encoder
	PolarCode PC(L, designSNR);

	int nBytes = (PCparam_K-8)>>3;

	default_random_engine RndGen;
	uniform_int_distribution<unsigned char> RndDist(0, 255);
	vector<vector<bool>> data(MaxIters, vector<bool>(PCparam_K-8,false));
	vector<bool> encodedData(PCparam_N, false);
	
	vector<vector<float>> sig(MaxIters, vector<float>(PCparam_N, 0.0));
	vector<float> LLR(PCparam_N,0.0);
	vector<bool> decodedData(PCparam_N,false);
	
	for(int block = 0; block<MaxIters; ++block)
	{
		//Generate random payload for testing
		for(int i=0; i<nBytes; ++i)
		{
			data[block][i] = RndDist(RndGen);
		} 
			
		//Encode
		PC.encode(encodedData, data[block]);
			
		//Modulate using simple BPSK
		modulate(sig[block], encodedData);
		
		//Distort / Transmit via AWGN-channel
		//(Signal+Noise) is normalized to noise power of 1
		// afterwards it is equalized for some perfectly known rayleigh fading channel
		std::normal_distribution<float> NormDist(0.0, 1.0);
		float factor = sqrt(R) * pow(10.0, EbN0/20.0)  * sqrt(2.0);
//		std::complex<float> channelCoeff, noise, sample;
		
		for(int i=0; i<PCparam_N; ++i)
		{
/*			sample.real(sig[block][i]*factor);
			sample.imag(0.0);
			channelCoeff.real(NormDist(RndGen));
			channelCoeff.imag(NormDist(RndGen));
			noise.real(NormDist(RndGen));
			noise.imag(NormDist(RndGen));
			
			noise /= channelCoeff;
			sample += noise;
			sig[block][i] = sample.real();*/
			sig[block][i] = sig[block][i]*factor + NormDist(RndGen);
		}
			
	}

	sprintf(message, "[%3d] Decoding with L = %d\n", SimIndex, L);
	std::cout << message;

	high_resolution_clock::time_point TimeStart = high_resolution_clock::now();
	
	while(runs < MaxIters && !(errors>=MinErrors && runs>=MinIters))
	{
		//Demodulate
		softDemod(LLR, sig[runs], R, EbN0);
		

		//Decode
		if(!PC.decode(decodedData, LLR))
		{
			++errors;
		}
		else
		{
			if(decodedData != data[runs])
			{
				++errors;
			}
		}
		++runs;
	}
	
	high_resolution_clock::time_point TimeEnd = high_resolution_clock::now();
	duration<float> TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart);
	float time = TimeUsed.count();
	
	Graph[SimIndex].runs = runs;
	Graph[SimIndex].errors = errors;
	Graph[SimIndex].BLER = errors;
	Graph[SimIndex].BLER /= runs;
	Graph[SimIndex].time = time;
	Graph[SimIndex].blps = runs;
	Graph[SimIndex].cbps = runs*PCparam_N;
	Graph[SimIndex].pbps = runs*(PCparam_K-8);
	Graph[SimIndex].blps /= time;
	Graph[SimIndex].cbps /= time;
	Graph[SimIndex].pbps /= time;
	Graph[SimIndex].effectiveRate = (runs-errors+0.0)*(PCparam_K-8.0)/time;
	
	
	
	if(runs == MaxIters && EbN0 < stopSNR[L])
	{
		stopSNR[L] = EbN0;
	}
	
	int finished = ++finishedThreads;
	sprintf(message, "[%3d] %3d Threads finished\n", SimIndex, finished);
	std::cout << message;
	
	--runningThreads;
}


int main(int argc, char** argv)
{
	int Sizes[] = {1, 2, 4, 8, 16}, nSizes = 5;
	Graph = new DataPoint[EbN0_count*nSizes];
	std::vector<std::thread> Threads;
	
	
/*	
	vector<float> vec;
	vec.push_back(-1.0);
	vec.push_back(-INFINITY);
	vec.push_back(-0.1);
	vec.push_back(-0.4);
	
	trackingSorter sorter;
	sorter.set(vec);
	sorter.sort();
	for(int i=0; i<4; ++i)
	{
		std::cout << sorter.permuted[i] << " => " << sorter.sorted[i] << std::endl;
	}*/
	
	

	std::ofstream File("../results/SimulatedDataAdaptivePCC,N=128,K=64+8.csv");
	if(!File.is_open())
	{
		std::cout << "Error opening the file!" << std::endl;
		return 0;
	}


	int idCounter = 0;
	for(int l=0; l<nSizes; ++l)
	{
		stopSNR[Sizes[l]] = INFINITY;
		for(int i=0; i<EbN0_count; ++i)
		{
			Graph[idCounter].EbN0 = EbN0_min + (EbN0_max-EbN0_min)/(EbN0_count-1)*i;
			Graph[idCounter].L = Sizes[l];
#ifndef __DEBUG__
			Threads.push_back(std::thread(simulate, idCounter++));
#else
			simulate(idCounter++);
#endif
		}
	}

#ifndef __DEBUG__
	for(auto& Thr : Threads)
	{
		Thr.join();
	}
#endif
	
	File << "\"L\",""\"Eb/N0\", \"Rate\", \"Runs\", \"Errors\",\"Time\",\"Blockspeed\",\"Coded Bitrate\",\"Payload Bitrate\",\"Effective Payload Bitrate\"" << std::endl;

	for(int i=0; i<EbN0_count*nSizes; ++i)
	{
		File << Graph[i].L << ',' << Graph[i].EbN0 << ',';
		if(Graph[i].BLER>0.0)
		{
			File << Graph[i].BLER;
		}
		else
		{
			File << "\" \"";
		}
		File << ','
		     << Graph[i].runs << ',' << Graph[i].errors << ','
		     << Graph[i].time << ',' << Graph[i].blps << ','
		     << Graph[i].cbps << ',' << Graph[i].pbps << ',';
		if(Graph[i].effectiveRate != 0)
		{
			File << Graph[i].effectiveRate;
		}
		File << std::endl;
	}
	
	File.close();
	
	return 0;
}
