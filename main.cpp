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

#include "Parameters.h"

const long long BitsToSimulate	= 1e8;//Bits
const int ConcurrentThreads = 2;

const float EbN0_min =  0;
const float EbN0_max =  7;
const int EbN0_count = 20;

/* In the following, you can manually select between four different parameters
 * to be varied. After (un)commenting, do not forget to (un)comment the
 * respective parametrization near the end of this file and adapt the csv-output!
 */
 

/* Code length comparison
const float designSNR = 10.0*log10(-1.0 * log(0.5));//=-1.591745dB
int ParameterN[] = {64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768}, nParams = 10;
int ParameterK[] = {32, 64,  128, 256,  512, 1024, 2048, 4096,  8192, 16384};
int L = 1;
const bool useCRC = false;
*/
/* Design-SNR measurement
float designParam[] = {-1.59, 0.0, 2.0, 4.0, 6.0};
const int nParams = 5;
const int N = 1<<11;
const int K = floor(N *   0.9   / 8)*8; //+8;
const int L = 1;
const bool useCRC = false;
 */
/* List length comparison */
const float designSNR = 0.0; // 10.0*log10(-1.0 * log(0.5));//=-1.591745dB
const int N = 512;
const int K = floor(N * 1.0/2.0 /8.0)*8+8;
int ParameterL[] = {1, 2, 4, 8, 16}; const int nParams = 5;
const bool useCRC = true;

/* Rate comparison
const float designSNR = 1.0;//10.0*log10(-1.0 * log(0.5));//=-1.591745dB
const int nParams = 6;
const int N = 2048;
float ParameterR[] = {1.0/4.0, 1.0/2.0, 2.0/3.0, 3.0/4.0, 5.0/6.0, 0.9};
const int L = 1;
const bool useCRC = false;
 */
 
std::atomic<unsigned int> finishedThreads(0);
std::mutex threadMutex[ConcurrentThreads];
std::condition_variable threadCV[ConcurrentThreads];

#ifdef ACCELERATED_MONTECARLO
std::map<int, std::atomic<float>> stopSNR;
#endif

struct DataPoint
{
	//Codec-Parameters
	float designSNR, EbN0; int N,K,L; bool useCRC;

	//Simulation-Parameters
	int BlocksToSimulate;
	
	//Simulator information
	std::atomic<bool> StopDecoder, StopChecker;
	std::mutex preloadMutex;
	std::condition_variable preloadCV;

	//Statistics
	int runs, bits, errors, biterrors;
	float BLER, BER;
	float time;//in seconds
	float blps,cbps,pbps;//blocks/coded bits/payload bits per second
	float effectiveRate;
} *Graph;


void simulate(int SimIndex)
{
	using namespace std;
	using namespace std::chrono;

	char message[128];
	Graph[SimIndex].runs = 0;
	Graph[SimIndex].bits = 0;
	Graph[SimIndex].errors = 0;
	Graph[SimIndex].biterrors = 0;
	Graph[SimIndex].time = 0;

	++finishedThreads;

	unique_lock<mutex> thrlck(threadMutex[SimIndex%ConcurrentThreads]);
	threadCV[SimIndex%ConcurrentThreads].wait(thrlck);
	

#ifdef ACCELERATED_MONTECARLO
	if(Graph[SimIndex].EbN0 >= stopSNR[Graph[SimIndex].L])
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
		sprintf(message, "[%3d] Skipping Eb/N0 = %f dB, L = %d\n", SimIndex, Graph[SimIndex].EbN0, Graph[SimIndex].L);
		std::cout << message;
		Graph[SimIndex].BLER = 0;
		Graph[SimIndex].time = 0;
		Graph[SimIndex].blps = 0;
		Graph[SimIndex].cbps = 0;
		Graph[SimIndex].pbps = 0;
		Graph[SimIndex].effectiveRate = 0;

		int finished = ++finishedThreads;
		sprintf(message, "[%3d] %3d Threads finished\n", SimIndex, finished);
		std::cout << message << flush;

		threadCV[SimIndex%ConcurrentThreads].notify_one();

		return;
	}
#endif

	Graph[SimIndex].StopDecoder = false;
	Graph[SimIndex].StopChecker = false;

	sprintf(message, "[%3d] Simulating Eb/N0=%.2f dB, N=%d, K=%d, L=%d, %s CRC\n", SimIndex, Graph[SimIndex].EbN0, Graph[SimIndex].N, Graph[SimIndex].K, Graph[SimIndex].L, Graph[SimIndex].useCRC?"with":"without");
	std::cout << message << flush;




	unsigned char* data;
	float* LLR;
	unsigned char* decodedData;



	int L = Graph[SimIndex].L;
	int N = Graph[SimIndex].N;
	float designSNR = Graph[SimIndex].designSNR;//dB
	float EbN0 = Graph[SimIndex].EbN0;//dB
	float R = (float)Graph[SimIndex].K / Graph[SimIndex].N;

	int BlocksToSimulate = Graph[SimIndex].BlocksToSimulate;

	int nBits = Graph[SimIndex].K;
	if(Graph[SimIndex].useCRC)nBits-=8;

	aligned_float_vector encodedData(N);
	aligned_float_vector sig(N);
	float factor = sqrt(pow(10.0, EbN0/10.0)  * 2.0 * R);

	PolarCode PC(N, Graph[SimIndex].K, L, Graph[SimIndex].useCRC, designSNR);

	//Allocate memory
	try{
		data = new unsigned char[Graph[SimIndex].K>>3];
		decodedData = new unsigned char[Graph[SimIndex].K>>3];
	}
	catch(bad_alloc &ba)
	{
		cerr << "Problem at allocating memory: " << ba.what() << endl;
		exit(EXIT_FAILURE);
	}
	memset(decodedData, 0, Graph[SimIndex].K>>3);
	LLR = (float*)_mm_malloc(N<<2, sizeof(vec));

	if(LLR == nullptr)
	{
		cerr << "_mm_malloc failed!" << endl;
		exit(EXIT_FAILURE);
	}

	//Data generation hints
	unsigned int* DataPtr = reinterpret_cast<unsigned int*>(data);
	unsigned int nInts = nBits>>5;
	unsigned int nRem = (nBits>>3)&3;
	unsigned int rawdata;

	//Bit error calculation hints
	int nBytes = nBits>>3;


	for(int b=0; b<BlocksToSimulate; ++b)
	{
		//Generate random payload
		for(unsigned int i=0; i<nInts; ++i)
		{
			_rdrand32_step(DataPtr+i);
		}
		{
			unsigned int offset = nInts<<2;
			_rdrand32_step(&rawdata);
			for(unsigned int j=0;j<nRem;++j)
			{
				data[offset|j] = static_cast<char>(rawdata&0xFF);
				rawdata >>= 8;
			}
		}

		//Encode
		PC.encode(encodedData, data);

		//Modulate using BPSK, add noise and demodulate
		PC.modulateAndDistort(LLR, encodedData, N, factor);


		//Decode and measure the required time
		high_resolution_clock::time_point TimeStart = high_resolution_clock::now();
		PC.decode(decodedData, LLR);
		high_resolution_clock::time_point TimeEnd = high_resolution_clock::now();

		duration<float> TimeUsed = duration_cast<duration<float>>(TimeEnd-TimeStart);
		Graph[SimIndex].time += TimeUsed.count();



		//Check for errors
		int biterrors = 0;
		for(int byte=0; byte < nBytes; ++byte)
		{
			unsigned char currentByteA = decodedData[byte],
						  currentByteB = data[byte],
						  cmpByte = currentByteA^currentByteB;
			if(cmpByte)
			{
				for(int bit=0; bit<8; ++bit)
				{
					biterrors += ((cmpByte)>>bit)&1;
				}
			}
		}

		Graph[SimIndex].runs++;
		Graph[SimIndex].bits += nBits;
		Graph[SimIndex].errors += !!biterrors;
		Graph[SimIndex].biterrors += biterrors;

	}


	delete [] data;
	delete [] decodedData;
	_mm_free(LLR);



	Graph[SimIndex].BLER = (float)Graph[SimIndex].errors/Graph[SimIndex].runs;
	Graph[SimIndex].BER = (float)Graph[SimIndex].biterrors/Graph[SimIndex].bits;
	//Graph[SimIndex].time = time;
	Graph[SimIndex].blps = Graph[SimIndex].runs;
	Graph[SimIndex].cbps = Graph[SimIndex].runs*Graph[SimIndex].N;
	Graph[SimIndex].pbps = Graph[SimIndex].bits;
	Graph[SimIndex].blps /= Graph[SimIndex].time;
	Graph[SimIndex].cbps /= Graph[SimIndex].time;
	Graph[SimIndex].pbps /= Graph[SimIndex].time;
	Graph[SimIndex].effectiveRate = (Graph[SimIndex].runs-Graph[SimIndex].errors+0.0)*nBits/Graph[SimIndex].time;

	
#ifdef ACCELERATED_MONTECARLO
	if(Graph[SimIndex].errors == 0 && EbN0 < stopSNR[L])
	{
		stopSNR[L] = EbN0;
	}
#endif

	int finished = ++finishedThreads;
	sprintf(message, "[%3d] %3d Threads finished, BLER = %e\n", SimIndex, finished, Graph[SimIndex].BLER);
	std::cout << message << flush;

	threadCV[SimIndex%ConcurrentThreads].notify_one();
}


int main(int argc, char** argv)
{
	Graph = new DataPoint[EbN0_count*nParams*2];
	std::vector<std::thread> Threads;
	
	std::ofstream File("Simulation.csv");
	if(!File.is_open())
	{
		std::cout << "Error opening the file!" << std::endl;
		return 0;
	}

	int idCounter = 0;
/*	for(int useCRC=0; useCRC<2; ++useCRC)
	{*/
	
		for(int l=0; l<nParams; ++l)
		{
		/* Code length comparison
			if(useCRC)
			{
				ParameterK[l] += 8;
			}
		*/
#ifdef ACCELERATED_MONTECARLO
			stopSNR[ParameterL[l]] = INFINITY;
#endif
			for(int i=0; i<EbN0_count; ++i)
			{
				Graph[idCounter].EbN0 = EbN0_min + (EbN0_max-EbN0_min)/(EbN0_count-1)*i;
			
			/* Code length comparison
				Graph[idCounter].N = ParameterN[l];
				Graph[idCounter].K = ParameterK[l];
				Graph[idCounter].L = (useCRC==1)?L:1;
				Graph[idCounter].designSNR = designSNR;
				Graph[idCounter].useCRC = useCRC;
			 */
			
			/* design-SNR measurement
			Graph[idCounter].N = N;
			Graph[idCounter].K = K;
			Graph[idCounter].L = L;
			Graph[idCounter].designSNR = designParam[l];
			Graph[idCounter].useCRC = useCRC;
			 */
				
			/* List length comparison */
			Graph[idCounter].N = N;
			Graph[idCounter].K = K;
			Graph[idCounter].L = ParameterL[l];
			Graph[idCounter].designSNR = designSNR;
			Graph[idCounter].useCRC = useCRC;

 			
			/* Rate comparison
			Graph[idCounter].N = N;
			Graph[idCounter].K = floor(N * ParameterR[l] / 8.0)*8 + (useCRC?8:0);
			Graph[idCounter].L = L;
			Graph[idCounter].designSNR = designSNR;
			Graph[idCounter].useCRC = useCRC;
			*/
 			
				Graph[idCounter].BlocksToSimulate = BitsToSimulate/  N /* ParameterN[l]*/;

				Threads.push_back(std::thread(simulate, idCounter++));
			}
		}
	//} CRC

	//Wait some time to let all threads lock up
	while(finishedThreads != Threads.size())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	finishedThreads = 0;
	for(int i=0; i<ConcurrentThreads; ++i)
	{
		//And then release the first $ConcurrentThreads threads
		threadCV[i].notify_one();
	}

	//Wait for all threads to return
	for(auto& Thr : Threads)
	{
		Thr.join();
	}
	
//	File << "\"N\",\"Eb/N0\",\"BLER\",\"BER\",\"Runs\",\"Errors\",\"Time\",\"Blockspeed\",\"Coded Bitrate\",\"Payload Bitrate\",\"Effective Payload Bitrate\"" << std::endl;
//	File << "\"designSNR\",\"Eb/N0\",\"BLER\",\"BER\",\"Runs\",\"Errors\",\"Time\",\"Blockspeed\",\"Coded Bitrate\",\"Payload Bitrate\",\"Effective Payload Bitrate\"" << std::endl;
	File << "\"L\",\"Eb/N0\",\"BLER\",\"BER\",\"Runs\",\"Errors\",\"Time\",\"Blockspeed\",\"Coded Bitrate\",\"Payload Bitrate\",\"Effective Payload Bitrate\"" << std::endl;
//	File << "\"R\",\"Eb/N0\",\"BLER\",\"BER\",\"Runs\",\"Errors\",\"Time\",\"Blockspeed\",\"Coded Bitrate\",\"Payload Bitrate\",\"Effective Payload Bitrate\"" << std::endl;

	for(int i=0; i<EbN0_count*nParams; ++i)
	{
//		File << Graph[i].N;
//		File << Graph[i].designSNR;
		File << Graph[i].L;
//		File << ((float)Graph[i].K/Graph[i].N);

		File << ',' << Graph[i].EbN0 << ',';
		if(Graph[i].BLER>0.0)
		{
			File << Graph[i].BLER << ',';
		}
		else
		{
			File << "nan,";
		}
		if(Graph[i].BER>0.0)
		{
			File << Graph[i].BER << ',';
		}
		else
		{
			File << "nan,";
		}
		File << Graph[i].runs << ',' << Graph[i].errors << ','
			 << Graph[i].time << ',' << Graph[i].blps << ',';
		if(Graph[i].cbps>0)
		{
			File << Graph[i].cbps << ',';
		}
		else
		{
			File << "nan,";
		}
		if(Graph[i].pbps>0)
		{
			File << Graph[i].pbps << ',';
		}
		else
		{
			File << "nan,";
		}
		if(Graph[i].effectiveRate != 0)
		{
			File << Graph[i].effectiveRate;
		}
		else
		{
			File << "nan";
		}
		File << std::endl;
	}
	
	File.close();

	return 0;
}
