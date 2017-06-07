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
#include <string>
#include <thread>
#include <vector>

#include "ArrayFuncs.h"
#include "PolarCode.h"

#include "Parameters.h"

const long long BitsToSimulate	= 4e8;//Bits
const int ConcurrentThreads = 3;

const float EbN0_min =  -1.6;
const float EbN0_max =   7.0;
const int EbN0_count =  20;

/* Code length comparison */
namespace codeLength {
	const float designSNR = 10.0*log10(-1.0 * log(0.5));//=-1.591745dB
	const int ParameterN[] = {32, 64, 128, 256, 512, 4096}, nParams = 6;
	const int ParameterK[] = {16, 32, 64,  128, 256, 2048};
	const int L = 1;
	const bool useCRC = false;
	const bool systematic = true;
}

/* Design-SNR measurement */
namespace designSNR {
	float designParam[] = {-1.591745, 0.0, 2.0, 4.0, 6.0, 8.0, 10.0};
	int nParams = 7;
	int N = 1<<7;
	int K = floor(N *   0.5   / 8)*8; //+8;
	int L = 1;
	bool useCRC = false;
	bool systematic = true;
}

/* List length comparison*/
namespace listLength {
	int N = 1<<7;
	bool useCRC = true;
	bool systematic = true;
	int K = floor(N * 1.0/2.0 /8.0)*8+(useCRC?8:0);
	float designSNR = 10.0*log10(-1.0 * log(0.5));//=-1.591745dB
	int ParameterL[] = {1, 2, 4, 8, 16, 32}; const int nParams = 5;
}

/* Rate comparison */
namespace rate {
	float designSNR = 0.0; //10.0*log10(-1.0 * log(0.5));//=-1.591745dB
	int nParams = 6;
	int N = 1<<12;
	float ParameterR[] = {1.0/4.0, 1.0/2.0, 2.0/3.0, 3.0/4.0, 5.0/6.0, 0.9};
	int L = 1;
	bool useCRC = false;
	bool systematic = true;
}
 
std::atomic<unsigned int> finishedThreads(0);
unsigned int totalThreads;
std::mutex threadMutex[ConcurrentThreads];
std::condition_variable threadCV[ConcurrentThreads];

#ifdef ACCELERATED_MONTECARLO
std::map<int, std::atomic<float>> stopSNR;
#endif

struct DataPoint
{
	//Codec-Parameters
	float designSNR, EbN0; int N,K,L; bool useCRC, systematic;

	//Simulation-Parameters
	int BlocksToSimulate;
	
	//Simulator information
	std::mutex preloadMutex;
	std::condition_variable preloadCV;

	//Statistics
	int runs, bits, errors, biterrors;
	float BLER, BER;
	float time;//in seconds
	float blps,cbps,pbps;//blocks/coded bits/payload bits per second
	float effectiveRate;
} *Graph[4];

struct {
#ifdef __RDRAND__
	void get(uint32_t *ptr)
	{
		_rdrand32_step(ptr);
	}
	void get64(uint64_t *ptr)
	{
		_rdrand64_step(ptr);
	}
#else
	LCG<uint64_t> generator;
	std::mutex mtx;
	void get(uint32_t *ptr)
	{
		mtx.lock();
		*ptr = static_cast<uint32_t>(generator());
		mtx.unlock();
	}
	void get64(uint64_t *ptr)
	{
		mtx.lock();
		*ptr = generator();
		mtx.unlock();
	}
#endif
} myRandomDevice;


const __m256 twopi = _mm256_set1_ps(2.0f * 3.14159265358979323846f);
const __m256 one = _mm256_set1_ps(1.0f);
const __m256 minustwo = _mm256_set1_ps(-2.0f);

void simulate(DataPoint* Graph, int SimIndex)
{
	using namespace std;
	using namespace std::chrono;

	std::string message;
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

	message.clear();
	message += "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
	message += "[";
	message += std::to_string(SimIndex);
	message += "] Simulating Eb/N0=";
	message += std::to_string(Graph[SimIndex].EbN0);
	message += " dB, N=";
	message += std::to_string(Graph[SimIndex].N);
	message += ", K=";
	message += std::to_string(Graph[SimIndex].K);
	message += ", L=";
	message += std::to_string(Graph[SimIndex].L);
	message += ", ";
	message += (Graph[SimIndex].useCRC?"with":"without");
	message += " CRC\n";

	message += "Progress: ";
	message += std::to_string(static_cast<int>(finishedThreads*100.0/totalThreads));
	message += "%";
	std::cout << message << flush;



	unsigned char* data;
	float* LLR;
	unsigned char* decodedData;



	int L = Graph[SimIndex].L;
	int N = Graph[SimIndex].N;
	float EbN0 = Graph[SimIndex].EbN0;//dB
	float R = (float)Graph[SimIndex].K / Graph[SimIndex].N;

	int BlocksToSimulate = Graph[SimIndex].BlocksToSimulate;

	int nBits = Graph[SimIndex].K;
//	if(Graph[SimIndex].useCRC)nBits-=8;

	aligned_float_vector encodedData(N);
	float factor = sqrt(pow(10.0, EbN0/10.0) * 2.0 * R);
	vec facVec = set1_ps(factor);

	PolarCode PC(N, Graph[SimIndex].K, L, Graph[SimIndex].useCRC, Graph[SimIndex].designSNR, Graph[SimIndex].systematic);

	//Allocate memory
	data = new unsigned char[Graph[SimIndex].K>>3];
	decodedData = new unsigned char[Graph[SimIndex].K>>3]();

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

	//Thread-local random device
	LCG<__m256> *r;
	uint64_t randomseed[4];

	myRandomDevice.get64(randomseed+0);
	myRandomDevice.get64(randomseed+1);
	myRandomDevice.get64(randomseed+2);
	myRandomDevice.get64(randomseed+3);

	r = (LCG<__m256>*)_mm_malloc(sizeof(LCG<__m256>), 32);
	r->seed(_mm256_set_epi64x(randomseed[0], randomseed[1], randomseed[2], randomseed[3]));


	for(int b=0; b<BlocksToSimulate; ++b)
	{
		//Generate random payload
		for(unsigned int i=0; i<nInts; ++i)
		{
			myRandomDevice.get(DataPtr+i);
		}
		{
			unsigned int offset = nInts<<2;
			myRandomDevice.get(&rawdata);
			for(unsigned int j=0;j<nRem;++j)
			{
				data[offset|j] = static_cast<char>(rawdata&0xFF);
				rawdata >>= 8;
			}
		}

		//Encode
		PC.encode(encodedData, data);

		//Modulate using BPSK, add noise and demodulate
		if(N < 16)
		{
			cerr << "Minimum block length is 16." << endl << flush;
			exit(EXIT_FAILURE);
		}

		for(int i=0; i<N; i+=16)
		{
			vec siga = load_ps(encodedData.data()+i);
			vec sigb = load_ps(encodedData.data()+i+8);
			siga = or_ps(siga, one); sigb = or_ps(sigb, one);//Modulate
			siga = mul_ps(siga, facVec); sigb = mul_ps(sigb, facVec);//Scale

			//Generate Gaussian noise
			__m256 u1 = _mm256_sub_ps(one, (*r)()); // [0, 1) -> (0, 1]
			__m256 u2 = (*r)();
			__m256 radius = _mm256_sqrt_ps(_mm256_mul_ps(minustwo, log256_ps(u1)));
			__m256 theta = _mm256_mul_ps(twopi, u2);
			__m256 sintheta, costheta;
			sincos256_ps(theta, &sintheta, &costheta);

			//Add noise to signal
#ifdef __FMA__
			siga = _mm256_fmadd_ps(radius, costheta, siga);
			sigb = _mm256_fmadd_ps(radius, sintheta, sigb);
#else
			siga = add_ps(mul_ps(radius, costheta), siga);
			sigb = add_ps(mul_ps(radius, sintheta), sigb);
#endif

			//Demodulate
			siga = mul_ps(siga, facVec);
			sigb = mul_ps(sigb, facVec);

			//Save
			store_ps(LLR+i, siga);
			store_ps(LLR+i+8, sigb);
		}



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
	_mm_free(r);


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
	message.clear();
	message += "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
	message += "[";
	message += std::to_string(SimIndex);
	message += "] ";
	message += std::to_string(finished);
	message += " Threads finished, BLER = ";
	message += std::to_string(Graph[SimIndex].BLER);
	message += "\n";
	message += "Progress: ";
	message += std::to_string(static_cast<int>(finishedThreads*100.0/totalThreads));
	message += "%";
	std::cout << message << flush;

	threadCV[SimIndex%ConcurrentThreads].notify_one();
}


int main(int argc, char** argv)
{
	int idCounter;
	Graph[0] = new DataPoint[EbN0_count*codeLength::nParams];
	Graph[1] = new DataPoint[EbN0_count*listLength::nParams];
	Graph[2] = new DataPoint[EbN0_count*designSNR::nParams];
	Graph[3] = new DataPoint[EbN0_count*rate::nParams];

	std::vector<std::thread> Threads;
	
	std::ofstream File[4];
	File[0].open("Simulation_codeLength.csv");
	File[1].open("Simulation_listLength.csv");
	File[2].open("Simulation_designSNR.csv");
	File[3].open("Simulation_rate.csv");

	/* Code length comparison */
	idCounter = 0;
	for(int p=0; p<codeLength::nParams; ++p)
	{
		for(int i=0; i<EbN0_count; ++i)
		{
			Graph[0][idCounter].EbN0 = EbN0_min + (EbN0_max-EbN0_min)/(EbN0_count-1)*p;
			Graph[0][idCounter].N = codeLength::ParameterN[p];
			Graph[0][idCounter].K = codeLength::ParameterK[p];
			Graph[0][idCounter].L = codeLength::L;
			Graph[0][idCounter].designSNR = codeLength::designSNR;
			Graph[0][idCounter].useCRC = codeLength::useCRC;
			Graph[0][idCounter].systematic = codeLength::systematic;
			Graph[0][idCounter].BlocksToSimulate = BitsToSimulate / codeLength::ParameterN[p];
			Threads.push_back(std::thread(simulate, Graph[0], idCounter++));
		}
	}

	/* List length comparison*/
	idCounter = 0;
	for(int p=0; p<listLength::nParams; ++p)
	{
		for(int i=0; i<EbN0_count; ++i)
		{
			Graph[1][idCounter].EbN0 = EbN0_min + (EbN0_max-EbN0_min)/(EbN0_count-1)*p;
			Graph[1][idCounter].N = listLength::N;
			Graph[1][idCounter].K = listLength::K;
			Graph[1][idCounter].L = listLength::ParameterL[p];
			Graph[1][idCounter].designSNR = listLength::designSNR;
			Graph[1][idCounter].useCRC = listLength::useCRC;
			Graph[1][idCounter].systematic = listLength::systematic;
			Graph[1][idCounter].BlocksToSimulate = BitsToSimulate / listLength::N;
			Threads.push_back(std::thread(simulate, Graph[1], idCounter++));
		}
	}

	/* design-SNR measurement */
	idCounter = 0;
	for(int p=0; p<designSNR::nParams; ++p)
	{
		for(int i=0; i<EbN0_count; ++i)
		{
			Graph[2][idCounter].EbN0 = EbN0_min + (EbN0_max-EbN0_min)/(EbN0_count-1)*p;
			Graph[2][idCounter].N = designSNR::N;
			Graph[2][idCounter].K = designSNR::K;
			Graph[2][idCounter].L = designSNR::L;
			Graph[2][idCounter].designSNR = designSNR::designParam[p];
			Graph[2][idCounter].useCRC = designSNR::useCRC;
			Graph[2][idCounter].systematic = designSNR::systematic;
			Graph[2][idCounter].BlocksToSimulate = BitsToSimulate / designSNR::N;
			Threads.push_back(std::thread(simulate, Graph[2], idCounter++));
		}
	}

	/* Rate comparison */
	idCounter = 0;
	for(int p=0; p<rate::nParams; ++p)
	{
		for(int i=0; i<EbN0_count; ++i)
		{
			Graph[3][idCounter].EbN0 = EbN0_min + (EbN0_max-EbN0_min)/(EbN0_count-1)*p;
			Graph[3][idCounter].N = rate::N;
			Graph[3][idCounter].K = floor(rate::N * rate::ParameterR[p] / 8.0)*8 + (rate::useCRC?8:0);
			Graph[3][idCounter].L = rate::L;
			Graph[3][idCounter].designSNR = rate::designSNR;
			Graph[3][idCounter].useCRC = rate::useCRC;
			Graph[3][idCounter].systematic = rate::systematic;
			Graph[3][idCounter].BlocksToSimulate = BitsToSimulate / rate::N;
			Threads.push_back(std::thread(simulate, Graph[3], idCounter++));
		}
	}

	totalThreads = Threads.size();

	std::cout << "Thread count: " << totalThreads << endl;

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

	std::cout << endl;

	File[0] << "\"N\",\"Eb/N0\",\"BLER\",\"BER\",\"Runs\",\"Errors\",\"Time\",\"Blockspeed\",\"Coded Bitrate\",\"Payload Bitrate\",\"Effective Payload Bitrate\"" << std::endl;
	File[1] << "\"L\",\"Eb/N0\",\"BLER\",\"BER\",\"Runs\",\"Errors\",\"Time\",\"Blockspeed\",\"Coded Bitrate\",\"Payload Bitrate\",\"Effective Payload Bitrate\"" << std::endl;
	File[2] << "\"designSNR\",\"Eb/N0\",\"BLER\",\"BER\",\"Runs\",\"Errors\",\"Time\",\"Blockspeed\",\"Coded Bitrate\",\"Payload Bitrate\",\"Effective Payload Bitrate\"" << std::endl;
	File[3] << "\"R\",\"Eb/N0\",\"BLER\",\"BER\",\"Runs\",\"Errors\",\"Time\",\"Blockspeed\",\"Coded Bitrate\",\"Payload Bitrate\",\"Effective Payload Bitrate\"" << std::endl;

	for(int i=0; i<EbN0_count*codeLength::nParams; ++i)
	{
		File[0] << Graph[0][i].N;

		File[0] << ',' << Graph[0][i].EbN0 << ',';
		if(Graph[0][i].BLER > 0.0)         File[0] << Graph[0][i].BLER << ',';   else File[0] << "nan,";
		if(Graph[0][i].BER  > 0.0)         File[0] << Graph[0][i].BER  << ',';   else File[0] << "nan,";
		File[0] << Graph[0][i].runs << ',' << Graph[0][i].errors << ',' << Graph[0][i].time << ',' << Graph[0][i].blps << ',';
		if(Graph[0][i].cbps > 0)           File[0] << Graph[0][i].cbps << ',';   else File[0] << "nan,";
		if(Graph[0][i].pbps > 0)           File[0] << Graph[0][i].pbps << ',';   else File[0] << "nan,";
		if(Graph[0][i].effectiveRate != 0) File[0] << Graph[0][i].effectiveRate; else File[0] << "nan";
		File[0] << std::endl;
	}
	for(int i=0; i<EbN0_count*listLength::nParams; ++i)
	{
		File[1] << Graph[1][i].L;

		File[1] << ',' << Graph[1][i].EbN0 << ',';
		if(Graph[1][i].BLER > 0.0)         File[1] << Graph[1][i].BLER << ',';   else File[1] << "nan,";
		if(Graph[1][i].BER  > 0.0)         File[1] << Graph[1][i].BER  << ',';   else File[1] << "nan,";
		File[1] << Graph[1][i].runs << ',' << Graph[1][i].errors << ',' << Graph[1][i].time << ',' << Graph[1][i].blps << ',';
		if(Graph[1][i].cbps > 0)           File[1] << Graph[1][i].cbps << ',';   else File[1] << "nan,";
		if(Graph[1][i].pbps > 0)           File[1] << Graph[1][i].pbps << ',';   else File[1] << "nan,";
		if(Graph[1][i].effectiveRate != 0) File[1] << Graph[1][i].effectiveRate; else File[1] << "nan";
		File[1] << std::endl;
	}
	for(int i=0; i<EbN0_count*designSNR::nParams; ++i)
	{
		File[2] << Graph[2][i].designSNR;

		File[2] << ',' << Graph[2][i].EbN0 << ',';
		if(Graph[2][i].BLER > 0.0)         File[2] << Graph[2][i].BLER << ',';   else File[2] << "nan,";
		if(Graph[2][i].BER  > 0.0)         File[2] << Graph[2][i].BER  << ',';   else File[2] << "nan,";
		File[2] << Graph[2][i].runs << ',' << Graph[2][i].errors << ',' << Graph[2][i].time << ',' << Graph[2][i].blps << ',';
		if(Graph[2][i].cbps > 0)           File[2] << Graph[2][i].cbps << ',';   else File[2] << "nan,";
		if(Graph[2][i].pbps > 0)           File[2] << Graph[2][i].pbps << ',';   else File[2] << "nan,";
		if(Graph[2][i].effectiveRate != 0) File[2] << Graph[2][i].effectiveRate; else File[2] << "nan";
		File[2] << std::endl;
	}
	for(int i=0; i<EbN0_count*rate::nParams; ++i)
	{
		File[3] << ((float)Graph[3][i].K/Graph[3][i].N);

		File[3] << ',' << Graph[3][i].EbN0 << ',';
		if(Graph[3][i].BLER > 0.0)         File[3] << Graph[3][i].BLER << ',';   else File[3] << "nan,";
		if(Graph[3][i].BER  > 0.0)         File[3] << Graph[3][i].BER  << ',';   else File[3] << "nan,";
		File[3] << Graph[3][i].runs << ',' << Graph[3][i].errors << ',' << Graph[3][i].time << ',' << Graph[3][i].blps << ',';
		if(Graph[3][i].cbps > 0)           File[3] << Graph[3][i].cbps << ',';   else File[3] << "nan,";
		if(Graph[3][i].pbps > 0)           File[3] << Graph[3][i].pbps << ',';   else File[3] << "nan,";
		if(Graph[3][i].effectiveRate != 0) File[3] << Graph[3][i].effectiveRate; else File[3] << "nan";
		File[3] << std::endl;
	}

	File[0].close();
	File[1].close();
	File[2].close();
	File[3].close();

	return 0;
}
