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

const int BufferInterval   =    1000;
const int MaxBufferSize    =   10000;
const int BlocksToSimulate =  100000;
const int ConcurrentThreads = 2;

#ifdef FLEXIBLE_DECODING
const int PCparam_N = 1024;
const int PCparam_K =  520;
const float designSNR = 5.0;
#else
#include "SpecialDecoderParams.h"
#endif


std::atomic<int> finishedThreads(0);
std::mutex threadMutex;
std::condition_variable threadCV;

#ifdef ACCELERATED_MONTECARLO
std::map<int, std::atomic<float>> stopSNR;
#endif

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
	std::condition_variable cv;
};

struct DataPoint
{
	float designSNR, EbN0; int N,K,L;
	int runs, bits, errors, biterrors;
	float BLER, BER;
	float time;//in seconds
	float blps,cbps,pbps;//blocks/coded bits/payload bits per second
	float effectiveRate;
	Buffer SignalBuffer, CheckBuffer;
	std::atomic<bool> StopDecoder, StopChecker;
	std::mutex preloadMutex;
	std::condition_variable preloadCV;
} *Graph;

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

	PolarCode PC(N, Graph[SimIndex].K, L, designSNR, true);
	
	Buffer mySigBuf;
	mySigBuf.ptr = nullptr;
	mySigBuf.size = 0;
	
	bool bufferFilled = false;

	for(int b=0; b<BlocksToSimulate; ++b)
	{
		if(Graph[SimIndex].SignalBuffer.size >= MaxBufferSize)
		{
			std::unique_lock<std::mutex> lck(Graph[SimIndex].SignalBuffer.mtx);
			Graph[SimIndex].SignalBuffer.cv.wait(lck);
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
			exit(EXIT_FAILURE);
		}
		memset(block->decodedData, 0, Graph[SimIndex].K<<2);
		block->LLR = (float*)_mm_malloc(N<<2, sizeof(vec));
		
		if(block->LLR == nullptr)
		{
			cerr << "_mm_malloc failed!" << endl;
			exit(EXIT_FAILURE);
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
			std::unique_lock<std::mutex> lck(Graph[SimIndex].SignalBuffer.mtx);
			Block* ptr = Graph[SimIndex].SignalBuffer.ptr;
			if(ptr == nullptr)
			{
				Graph[SimIndex].SignalBuffer.ptr = mySigBuf.ptr;
			}
			else
			{
				//Find the last element of the smaller stack
				//to put the bigger one onto it
				//(the order of the blocks is irrelevant for this simulation)
				if(Graph[SimIndex].SignalBuffer.size < mySigBuf.size)
				{
					while(ptr->next != nullptr)
						ptr = ptr->next;
					ptr->next = mySigBuf.ptr;
				}
				else
				{
					ptr = mySigBuf.ptr;
					while(ptr->next != nullptr)
						ptr = ptr->next;
					ptr->next = Graph[SimIndex].SignalBuffer.ptr;
					Graph[SimIndex].SignalBuffer.ptr = mySigBuf.ptr;
				}
			}			
			Graph[SimIndex].SignalBuffer.size += (int)mySigBuf.size;
			mySigBuf.ptr = nullptr;
			mySigBuf.size = 0;
			if(Graph[SimIndex].SignalBuffer.size >= MaxBufferSize && !bufferFilled)
			{
				std::unique_lock<std::mutex> plck(Graph[SimIndex].preloadMutex);
				Graph[SimIndex].preloadCV.notify_one();
				bufferFilled = true;
			}
		}
	}
	Graph[SimIndex].StopDecoder = true;
}

void decodeSignals(int SimIndex)
{
	PolarCode PC(Graph[SimIndex].N, Graph[SimIndex].K, Graph[SimIndex].L, Graph[SimIndex].designSNR);

	Buffer mySigBuf, myChkBuf;
	mySigBuf.ptr = nullptr;
	mySigBuf.size = 0;
	myChkBuf.ptr = nullptr;
	myChkBuf.size = 0;

	while(Graph[SimIndex].SignalBuffer.size || mySigBuf.size)
	{
		if(mySigBuf.ptr == nullptr)
		{
			while(Graph[SimIndex].SignalBuffer.ptr == nullptr && !Graph[SimIndex].StopDecoder)
			{
				//Buffer underrun!!!
				std::this_thread::yield();
			}
			std::unique_lock<std::mutex> lck(Graph[SimIndex].SignalBuffer.mtx);
			//Get $BufferInterval elements from generator
			mySigBuf.ptr = Graph[SimIndex].SignalBuffer.ptr;
			mySigBuf.size = (int)Graph[SimIndex].SignalBuffer.size;
			Graph[SimIndex].SignalBuffer.ptr = nullptr;
			Graph[SimIndex].SignalBuffer.size = 0;
			Graph[SimIndex].SignalBuffer.cv.notify_one();
		}
		if(Graph[SimIndex].StopDecoder && !mySigBuf.size)
		{
			break;
		}

		Block *block = mySigBuf.ptr;
		
		PC.decode(block->decodedData, block->LLR);
		
		
		Block *nextBlock = block->next;
		//Push block into Checker
		block->next = myChkBuf.ptr;
		myChkBuf.ptr = block;
		myChkBuf.size++;
		
		if(myChkBuf.size >= BufferInterval)
		{
			std::unique_lock<std::mutex> lck(Graph[SimIndex].CheckBuffer.mtx);
			Block *ptr = Graph[SimIndex].CheckBuffer.ptr;
			if(ptr != nullptr)
			{
				if(Graph[SimIndex].CheckBuffer.size < myChkBuf.size)
				{
					while(ptr->next != nullptr)
						ptr = ptr->next;
					ptr->next = myChkBuf.ptr;
				}
				else
				{
					ptr = myChkBuf.ptr;
					while(ptr->next != nullptr)
						ptr = ptr->next;
					ptr->next = Graph[SimIndex].CheckBuffer.ptr;
					Graph[SimIndex].CheckBuffer.ptr = myChkBuf.ptr;					
				}
			}
			else
			{
				Graph[SimIndex].CheckBuffer.ptr = myChkBuf.ptr;
			}
			Graph[SimIndex].CheckBuffer.size += myChkBuf.size;
			Graph[SimIndex].CheckBuffer.cv.notify_one();
			myChkBuf.ptr = nullptr;
			myChkBuf.size = 0;
		}
		
		//Get next block from Generator
		mySigBuf.ptr = nextBlock;
		mySigBuf.size--;
	}
	
	std::unique_lock<std::mutex> lck(Graph[SimIndex].CheckBuffer.mtx);
	Block *ptr = Graph[SimIndex].CheckBuffer.ptr;
	if(ptr != nullptr)
	{
		while(ptr->next != nullptr)
			ptr = ptr->next;
		ptr->next = myChkBuf.ptr;
	}
	else
	{
		Graph[SimIndex].CheckBuffer.ptr = myChkBuf.ptr;
	}
	Graph[SimIndex].CheckBuffer.size += myChkBuf.size;
	Graph[SimIndex].StopChecker = true;
	Graph[SimIndex].CheckBuffer.cv.notify_one();
}

void checkDecodedData(int SimIndex)
{
#ifdef CRCSIZE
	int nBits = (Graph[SimIndex].K-CRCSIZE);
#else
	int nBits = Graph[SimIndex].K;
#endif

	Buffer myChkBuf;
	myChkBuf.ptr = nullptr;
	myChkBuf.size = 0;

	while(!Graph[SimIndex].StopChecker || Graph[SimIndex].CheckBuffer.size || myChkBuf.size)
	{
		if(myChkBuf.ptr == nullptr)
		{
			if(Graph[SimIndex].CheckBuffer.ptr == nullptr && !Graph[SimIndex].StopChecker)
			{
				std::unique_lock<std::mutex> lck(Graph[SimIndex].CheckBuffer.mtx);
				Graph[SimIndex].CheckBuffer.cv.wait(lck);
			}
			if(Graph[SimIndex].StopChecker && !Graph[SimIndex].CheckBuffer.size)return;
			
			//Pop block
			Graph[SimIndex].CheckBuffer.mtx.lock();
			myChkBuf.ptr = Graph[SimIndex].CheckBuffer.ptr;
			myChkBuf.size = (int)Graph[SimIndex].CheckBuffer.size;
			Graph[SimIndex].CheckBuffer.ptr = nullptr;
			Graph[SimIndex].CheckBuffer.size = 0;
			Graph[SimIndex].CheckBuffer.mtx.unlock();
		}
		
		Block* BlockPtr = myChkBuf.ptr;
		Block* nextBlock = myChkBuf.ptr->next;

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
		myChkBuf.ptr = nextBlock;
		myChkBuf.size--;
	}
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

/*
#ifdef ACCELERATED_MONTECARLO
	while(SimIndex > nextThread && EbN0 < stopSNR[L])
#else
	while(SimIndex > nextThread)
#endif
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
*/

	{
		std::unique_lock<std::mutex> thrlck(threadMutex);
		threadCV.wait(thrlck);
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

	Graph[SimIndex].SignalBuffer.ptr = nullptr;
	Graph[SimIndex].SignalBuffer.size = 0;
	Graph[SimIndex].CheckBuffer.ptr = nullptr;
	Graph[SimIndex].CheckBuffer.size = 0;
	Graph[SimIndex].StopDecoder = false;
	Graph[SimIndex].StopChecker = false;

	sprintf(message, "[%3d] Simulating Eb/N0 = %f dB, L = %d\n", SimIndex, Graph[SimIndex].EbN0, Graph[SimIndex].L);
	std::cout << message;

	
	thread Generator(generateSignals, SimIndex);

	{
		std::unique_lock<std::mutex> plck(Graph[SimIndex].preloadMutex);
		Graph[SimIndex].preloadCV.wait(plck);
	}

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
	Decoder.join();
	high_resolution_clock::time_point TimeEnd = high_resolution_clock::now();
	Generator.join();
	Checker.join();
	
	duration<float> TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart);
	float time = TimeUsed.count();

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

//	++nextThread;
	threadCV.notify_one();
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
//	nextThread = ConcurrentThreads-1;
	for(int i=0; i<ConcurrentThreads; ++i)
	{
		std::unique_lock<std::mutex> lck(threadMutex);
		threadCV.notify_one();
	}
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
