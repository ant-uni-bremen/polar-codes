#include <atomic>
#include <chrono>
#include <cmath>
#include <complex>
#include <condition_variable>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

#include "ArrayFuncs.h"
#include "PolarCode.h"
#include "Modem.h"

#include "Parameters.h"

const int BufferInterval   =  1000;
const int MaxBufferSize    = 10000;
const int BlocksToSimulate = 50000;
const int ConcurrentThreads = 1;

#ifdef FLEXIBLE_DECODING
const int PCparam_N = 1024;
const int PCparam_K =  520;
const float designSNR = 5.0;
#else
#include "SpecialDecoderParams.h"
#endif


std::atomic<int> nextThread(-1), finishedThreads(0);

#ifdef ACCELERATED_MONTECARLO
std::map<int, std::atomic<float>> stopSNR;
#endif


struct DataPoint
{
	float designSNR, EbN0; int N,K,L;
	int runs, bits, errors, biterrors;
	float BLER, BER;
	float time;//in seconds
	float blps,cbps,pbps;//blocks/coded bits/payload bits per second
	float effectiveRate;
} *Graph;

struct Block
{
	float* data;
	float* LLR;
	float* decodedData;
	Block* next;
};

struct Buffer {
	Block *ptr;
	std::atomic<int> size;
	std::mutex mtx;
} SignalBuffer, CheckBuffer;

std::atomic<bool> StopDecoder, bufferFilled;

void generateSignals(int SimIndex)
{
	int L = Graph[SimIndex].L;
	int N = Graph[SimIndex].N;
	float designSNR = Graph[SimIndex].designSNR;//dB
	float EbN0 = Graph[SimIndex].EbN0;//dB
	float R = (float)Graph[SimIndex].K / Graph[SimIndex].N;
#ifdef CRCSIZE
	int nBits = (Graph[SimIndex].K-CRCSIZE);
#else
	int nBits = Graph[SimIndex].K;
#endif
	aligned_float_vector encodedData(N);
	aligned_float_vector sig(N);
	float factor = sqrt(R) * pow(10.0, EbN0/20.0)  * sqrt(2.0);
	mt19937 RndGen(SimIndex);

	PolarCode PC(Graph[SimIndex].N, Graph[SimIndex].K, L, designSNR, true);
	
	Buffer mySigBuf;
	mySigBuf.ptr = nullptr;
	mySigBuf.size = 0;

//	cout << "Generator starts" << endl;

	for(int b=0; b<BlocksToSimulate; ++b)
	{
		while(SignalBuffer.size >= MaxBufferSize)
		{
			std::this_thread::yield();
		}
		
		Block *block = new Block;
		block->next = nullptr;
		
		try{
			block->data = new float[Graph[SimIndex].K];
			block->decodedData = new float[Graph[SimIndex].K];
		}
		catch(bad_alloc &ba)
		{
			cerr << "Problem at allocating memory: " << ba.what() << endl;
		}
		memset(block->decodedData, 0, Graph[SimIndex].K<<2);
		block->LLR = (float*)_mm_malloc(N<<2, sizeof(vec));
		
		if(block->LLR == nullptr)
		{
			cerr << "_mm_malloc failed!" << endl;
		}

		//Generate random payload for testing
		unsigned int* DataPtr = reinterpret_cast<unsigned int*>(block->data);
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
		PC.encode(encodedData, block->data);

		//Modulate using BPSK and add noise
		modulateAndDistort(sig, encodedData, N, factor);

		//Demodulate
		softDemod(block->LLR, sig, N, R, EbN0);
		
		//Add to private decoding queue
		block->next = mySigBuf.ptr;
		mySigBuf.ptr = block;
		mySigBuf.size++;
		
		if(mySigBuf.size >= BufferInterval || b+1 == BlocksToSimulate)
		{
			//Add to public decoding queue
			SignalBuffer.mtx.lock();
			Block* ptr = SignalBuffer.ptr;
			if(ptr == nullptr)
			{
				SignalBuffer.ptr = mySigBuf.ptr;
				SignalBuffer.size = mySigBuf.size+0;
			}
			else
			{
				while(ptr->next != nullptr)
					ptr = ptr->next;
				ptr->next = mySigBuf.ptr;
				SignalBuffer.size += mySigBuf.size;
			}			
			mySigBuf.ptr = nullptr;
			mySigBuf.size = 0;
			if(SignalBuffer.size >= MaxBufferSize && !bufferFilled)
			{
				bufferFilled = true;
			}
			SignalBuffer.mtx.unlock();
		}
	}
	StopDecoder = true;
//	cout << "Generator finished" << endl;
}

void decodeSignals(int SimIndex)
{
	PolarCode PC(Graph[SimIndex].N, Graph[SimIndex].K, Graph[SimIndex].L, Graph[SimIndex].designSNR);
	Block* BlockPtr = nullptr;
//	cout << "Decoder starts" << endl;

#warning TODO: Add private buffers to decoder and checker to reduce inter-thread communication time

	while(!StopDecoder || SignalBuffer.size)
	{
		if(BlockPtr == nullptr)
		{
			while(SignalBuffer.ptr == nullptr && !StopDecoder)
			{
				std::this_thread::yield();
			}
			SignalBuffer.mtx.lock();
			BlockPtr = SignalBuffer.ptr;
			SignalBuffer.ptr = SignalBuffer.ptr->next;
			SignalBuffer.size--,
			SignalBuffer.mtx.unlock();
		}
		if(StopDecoder && !SignalBuffer.size)
		{
			return;
		}

		Block *block = BlockPtr;
		
		PC.decode(block->decodedData, block->LLR);
		
		//Push block into Checker
		CheckBuffer.mtx.lock();
		block->next = CheckBuffer.ptr;
		CheckBuffer.ptr = block;
		CheckBuffer.size++;
		CheckBuffer.mtx.unlock();
		
		//Get next block from Generator
		SignalBuffer.mtx.lock();
		BlockPtr = SignalBuffer.ptr;
		if(BlockPtr != nullptr)
		{
			SignalBuffer.ptr = SignalBuffer.ptr->next;
			SignalBuffer.size--;
		}
		SignalBuffer.mtx.unlock();
	}
//	cout << "Decoder finished" << endl;
}

void checkDecodedData(int SimIndex)
{
#ifdef CRCSIZE
	int nBits = (Graph[SimIndex].K-CRCSIZE);
#else
	int nBits = Graph[SimIndex].K;
#endif

	Block* BlockPtr = nullptr;

//	cout << "Checker starts" << endl;
	while(!StopDecoder || CheckBuffer.size)
	{
		if(BlockPtr == nullptr)
		{
			while(CheckBuffer.ptr == nullptr && !StopDecoder)
			{
				std::this_thread::yield();
			}
			if(StopDecoder)return;
			
			//Pop block
			CheckBuffer.mtx.lock();
			BlockPtr = CheckBuffer.ptr;
			CheckBuffer.ptr = CheckBuffer.ptr->next;
			CheckBuffer.size--;
			CheckBuffer.mtx.unlock();
		}

		unsigned int *decData = reinterpret_cast<unsigned int*>(BlockPtr->decodedData);
		unsigned int *orgData = reinterpret_cast<unsigned int*>(BlockPtr->data);
		
		int biterrors = 0;
		for(int bit=0; bit<nBits; ++bit)
		{
			biterrors += (decData[bit]^orgData[bit])>>31;
		}

		Graph[SimIndex].runs++;
		Graph[SimIndex].bits += nBits;
		Graph[SimIndex].errors += !!biterrors;
		Graph[SimIndex].biterrors += biterrors;
		

		delete [] BlockPtr->data;
		delete [] BlockPtr->decodedData;
		_mm_free(BlockPtr->LLR);
		delete BlockPtr;
		
		//Get next block from Decoder
		CheckBuffer.mtx.lock();
		BlockPtr = CheckBuffer.ptr;
		if(BlockPtr != nullptr)
		{
			CheckBuffer.ptr = CheckBuffer.ptr->next;
			CheckBuffer.size--;
		}
		CheckBuffer.mtx.unlock();
	}
//	cout << "Checker finished" << endl;
}



void simulate(int SimIndex)
{
	using namespace std;
	using namespace std::chrono;

#ifdef CRCSIZE
	int nBits = (Graph[SimIndex].K-CRCSIZE);
#else
	int nBits = Graph[SimIndex].K;
#endif

	char message[128];
	
#ifdef ACCELERATED_MONTECARLO
	while(SimIndex > nextThread && EbN0 < stopSNR[L])
#else
	while(SimIndex > nextThread)
#endif
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	
	Graph[SimIndex].runs = 0;
	Graph[SimIndex].bits = 0;
	Graph[SimIndex].errors = 0;
	Graph[SimIndex].biterrors = 0;

#ifdef ACCELERATED_MONTECARLO
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
		Graph[SimIndex].BLER = 0;
		Graph[SimIndex].time = 0;
		Graph[SimIndex].blps = 0;
		Graph[SimIndex].cbps = 0;
		Graph[SimIndex].pbps = 0;
		Graph[SimIndex].effectiveRate = 0;
		++finishedThreads;
		++nextThread;
		
		return;
	}
#endif

	SignalBuffer.ptr = nullptr;
	SignalBuffer.size = 0;
	CheckBuffer.ptr = nullptr;
	CheckBuffer.size = 0;
	StopDecoder = false;
	bufferFilled = false;

	sprintf(message, "[%3d] Simulating Eb/N0 = %f dB, L = %d\n", SimIndex, Graph[SimIndex].EbN0, Graph[SimIndex].L);
	std::cout << message;

	
	thread Generator(generateSignals, SimIndex);
	while(!bufferFilled)
	{
		std::this_thread::yield();
	}
//	cout << "Start" << endl;
	high_resolution_clock::time_point TimeStart = high_resolution_clock::now();
	thread Decoder(decodeSignals, SimIndex);
	thread Checker(checkDecodedData, SimIndex);
	
/*	while(!StopDecoder)
	{
		this_thread::sleep_for(chrono::milliseconds(1000));
		BufferMutex.lock();
		cout << "SignalBuffer: " << SignalBuffer.size() << endl
			 << "CheckBuffer:  " << CheckBuffer.size() << endl
			 << endl;
		BufferMutex.unlock();
	}*/
	Generator.join();
	Decoder.join();
	Checker.join();
	
	high_resolution_clock::time_point TimeEnd = high_resolution_clock::now();
	duration<float> TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart);
	float time = TimeUsed.count();

	//Graph[SimIndex].runs = runs;
	//Graph[SimIndex].errors = errors;
	Graph[SimIndex].BLER = (float)Graph[SimIndex].errors/Graph[SimIndex].runs;
	Graph[SimIndex].BER = (float)Graph[SimIndex].biterrors/Graph[SimIndex].bits;
	Graph[SimIndex].time = time;
	Graph[SimIndex].blps = Graph[SimIndex].runs;
	Graph[SimIndex].cbps = Graph[SimIndex].runs*Graph[SimIndex].N;
	Graph[SimIndex].pbps = Graph[SimIndex].bits;
	Graph[SimIndex].blps /= time;
	Graph[SimIndex].cbps /= time;
	Graph[SimIndex].pbps /= time;
	Graph[SimIndex].effectiveRate = (Graph[SimIndex].runs-Graph[SimIndex].errors+0.0)*nBits/time;

	
#ifdef ACCELERATED_MONTECARLO
	if(Graph[SimIndex].runs == BlocksToSimulate && EbN0 < stopSNR[L])
	{
		stopSNR[L] = EbN0;
	}
#endif

	int finished = ++finishedThreads;
	sprintf(message, "[%3d] %3d Threads finished, BLER = %e\n", SimIndex, finished, Graph[SimIndex].BLER);
	std::cout << message;

	++nextThread;
}


int main(int argc, char** argv)
{
//	int Parameter[] = {1,2}, nParams = 2;
	int Parameter[] = {1, 2,4}, nParams = 3;
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
#ifdef ACCELERATED_MONTECARLO
		stopSNR[Parameter[l]] = INFINITY;
#endif
		for(int i=0; i<EbN0_count; ++i)
		{
			Graph[idCounter].EbN0 = EbN0_min + (EbN0_max-EbN0_min)/(EbN0_count-1)*i;
			Graph[idCounter].N = PCparam_N;
			Graph[idCounter].K = PCparam_K;
			Graph[idCounter].L = Parameter[l];
			Graph[idCounter].designSNR = designSNR;
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
		     << Graph[i].time << ',' << Graph[i].blps << ',';
		if(Graph[i].cbps>0)
		{
			File << Graph[i].cbps << ',';
		}
		else
		{
			File << "\" \",";
		}
		File << Graph[i].pbps << ',';
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
