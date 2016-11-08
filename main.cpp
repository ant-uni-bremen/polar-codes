#include <atomic>
#include <chrono>
#include <cmath>
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

#define EbN0_min  0.0
#define EbN0_max  6.0
#define EbN0_count 20

#define ConcurrentThreads 3

#define MinErrors    100
#define MinIters   10000
#define MaxIters   50000

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
	
	int N = 1<<7, K = (1<<6)+8, L = Graph[SimIndex].L;
	float designSNR = 0.0;//dB
	float EbN0 = Graph[SimIndex].EbN0;//dB
	
	float R = (float)K/N;
	
	int runs = 0, errors = 0;
	char message[128];
	
	while(runningThreads >= ConcurrentThreads/* && EbN0 < stopSNR[L]*/)
	{
//		std::this_thread::yield();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
/*	if(EbN0 >= stopSNR[L])
	{
*/		/* This might save some time.
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
	
	++runningThreads;
	
	sprintf(message, "[%3d] Launching for Eb/N0 = %f dB, L = %d\n", SimIndex, EbN0, L);
	std::cout << message;

	//Encoder
	PolarCode PC(N, K, L, designSNR);

	int nBytes = (K-8)>>3;

	default_random_engine RndGen;
	uniform_int_distribution<unsigned char> RndDist(0, 255);
	vector<vector<bool>> data(MaxIters, vector<bool>(K-8,false));
	vector<bool> encodedData(N, false);
	
	vector<vector<float>> sig(MaxIters, vector<float>(N, 0.0));
	vector<float> LLR(N,0.0);
	vector<bool> decodedData(N,false);
	
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
		std::normal_distribution<float> NormDist(0.0, 1.0);
		float factor = sqrt(R) * pow(10.0, EbN0/20.0)  * sqrt(2.0);
		
		for(int i=0; i<N; ++i)
		{
			sig[block][i] = sig[block][i]*factor + NormDist(RndGen);
		}
			
	}
	
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
	Graph[SimIndex].cbps = runs*N;
	Graph[SimIndex].pbps = runs*(K-8);
	Graph[SimIndex].blps /= time;
	Graph[SimIndex].cbps /= time;
	Graph[SimIndex].pbps /= time;
	Graph[SimIndex].effectiveRate = (runs-errors+0.0)*(K-8.0)/time;
	
	
	
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

	std::ofstream File("../results/SimulatedDataWBTCPP,N=128.csv");
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
		File << Graph[i].L << ','
		     << Graph[i].EbN0 << ',' << Graph[i].BLER << ','
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
