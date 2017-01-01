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

const int MaxIters = 100000;
const int ConcurrentThreads = 1;


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
	int N = Graph[SimIndex].N;
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

	mt19937 RndGen(1);
	uniform_int_distribution<unsigned char> RndDist(0, 1);
//	normal_distribution<float> NormDist(0.0, 1.0);
	vector<vector<float>> data(MaxIters, vector<float>(nBits,0.0));
	vector<vector<float>> decodedData(MaxIters, vector<float>(Graph[SimIndex].K,0.0));
	aligned_float_vector encodedData(N);
	
	aligned_float_vector sig(N,0.0);
	vector<aligned_float_vector> LLR(MaxIters, aligned_float_vector(N));
	//vector<bool> decodedData(PCparam_N,false);
	
	float factor = sqrt(R) * pow(10.0, EbN0/20.0)  * sqrt(2.0);
	
	for(int block = 0; block<MaxIters; ++block)
	{
		//Generate random payload for testing
		unsigned int* DataPtr = reinterpret_cast<unsigned int*>(data[block].data());
		for(int i=0; i<nBits; i+=32)
		{
			unsigned int rawdata = RndGen();
			for(int j=0;j<32;++j)
			{
				*(DataPtr++) = rawdata&0x80000000;
				rawdata <<= 1;
			}
		}

		//Encode
		PC.encode(encodedData, data[block]);
			
		//Modulate using BPSK and add noise
		modulateAndDistort(sig, encodedData, N, factor);
			
		//Demodulate
		softDemod(LLR[block], sig, N, R, EbN0);
	}

	sprintf(message, "[%3d] Decoding with L = %d\n", SimIndex, L);
	std::cout << message;

	high_resolution_clock::time_point TimeStart = high_resolution_clock::now();
	
	while(runs < MaxIters/* && !(errors>=MinErrors && runs>=MinIters)*/)
	{
		//Decode
		
/*		if(!*/PC.decode(decodedData[runs], LLR[runs]);/*)
		{
			++errors;
		}*/
		++runs;
	}
	
	high_resolution_clock::time_point TimeEnd = high_resolution_clock::now();
	duration<float> TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart);
	float time = TimeUsed.count();

	errors = 0;
	
	for(int block=0; block<runs; ++block)
	{
		bool errorFound = false;
		unsigned int* decData = reinterpret_cast<unsigned int*>(decodedData[block].data());
		unsigned int* orgData = reinterpret_cast<unsigned int*>(data[block].data());
		for(int i=0; i<nBits; ++i)
		{
			unsigned int a = decData[i];
			unsigned int b = orgData[i];
			if(a != b)
			{
				if(!errorFound)
				{
					++errors;
					errorFound = true;
				}
				++biterrors;
			}
		}
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
//	int Parameter[] = {1,2}, nParams = 2;
	int Parameter[] = {1,2,4}, nParams = 3;
//	int Parameter[] = {1, 2, 4, 8, 16}, nParams = 5;
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
			Graph[idCounter].N = 1024;
			Graph[idCounter].K = 520;
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
