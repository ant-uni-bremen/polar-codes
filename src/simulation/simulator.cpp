#include "simulator.h"

#include <thread>
#include <fstream>
#include <iostream>
#include <cmath>

#include <signalprocessing/modulation/ask.h>

#include <polarcode/encoding/butterfly_fip_packed.h>

#include <polarcode/decoding/fastssc_avx_float.h>
#include <polarcode/decoding/fastssc_fip_char.h>
#include <polarcode/decoding/scl_fip_char.h>
#include <polarcode/decoding/scl_avx_float.h>
#include <polarcode/decoding/adaptive_float.h>
#include <polarcode/decoding/adaptive_char.h>
#include <polarcode/decoding/adaptive_mixed.h>
#include <polarcode/decoding/fixed_fip_char.h>
#include <polarcode/decoding/depth_first.h>
#include <polarcode/decoding/scan.h>
#include <polarcode/decoding/fastsscan_float.h>

#include <polarcode/errordetection/dummy.h>
#include <polarcode/errordetection/crc8.h>
#include <polarcode/errordetection/crc32.h>

namespace Simulation {

Simulator::Simulator(Setup::Configurator *config)
	: mConfiguration(config)
	, mNextJob(0) {

	std::string simType = mConfiguration->getString("simtype");

	if(simType == "single") {
		configureSingleRun();
	} else if(simType == "codelength") {
		configureCodeLengthSim();
	} else if(simType == "designsnr") {
		configureDesignSnrSim();
	} else if(simType == "listlength") {
		configureListLengthSim();
	} else if(simType == "rate") {
		configureRateSim();
	} else if(simType == "amplification") {
		configureAmplificationSim();
	} else if(simType == "fixed") {
		configureFixedSim();
	} else if(simType == "depthfirst") {
		configureDepthFirstSim();
	} else if(simType == "scan") {
		configureScanSim(false);
	} else if(simType == "fastsscan") {
		configureScanSim(true);
	} else if(simType == "ask") {
		configureAskSim();
	} else if(simType == "compareall") {
		configureComparisonSim();
		return;// No SNR inflation
	} else if(simType == "getcode") {
		printCode();
	} else {
		// Unknown simulation type, should be caught by cmd-parser
		exit(EXIT_FAILURE);
	}
	snrInflateJobList();
}

Simulator::~Simulator() {
	for(auto Job : mJobList) {
		delete Job;
	}
}

void Simulator::run() {
	// Get number of concurrently active simulation threads
	unsigned threadCount = mConfiguration->getInt("threads");
	std::vector<std::thread> Threads;

	// Start the work
	try {
		for(unsigned i=0; i<threadCount; ++i) {
			Threads.push_back(std::thread(SimThread, this, i+1));
		}

		// Wait for all threads to stop
		for(auto& Thread : Threads) {
			Thread.join();
		}
	}
	catch(std::system_error e) {
		std::cout << "Cannot use multithreaded simulation!\n";
		std::cout << "what(): " << e.what() << std::endl;
		SimThread(this, 1);
	}



	// Write results into file
	if(mConfiguration->getString("simtype") == "compareall") {
		saveComparisonResults();
	} else {
		saveResults();
	}
}

DataPoint* Simulator::getJob() {
	unsigned jobId = mNextJob.fetch_add(1);

	if(jobId < mJobList.size()) {
		std::string message = "[0] Jobs in queue: ";
		message += std::to_string(mJobList.size()-jobId-1);
		message += "\n";
		std::cout << message;

		return mJobList[jobId];
	} else {
		return nullptr;
	}
}

DataPoint* Simulator::getDefaultDataPoint() {
	DataPoint* dp = new DataPoint();

	// Set code parameters
	dp->designSNR =          mConfiguration->getFloat("design-snr");
	dp->N =                  mConfiguration->getInt("blocklength");
	dp->K = dp->N *          mConfiguration->getFloat("rate");
	dp->L =                  mConfiguration->getInt("pathlimit");
	dp->errorDetection =     errorDetectionStringToId(mConfiguration->getString("error-detection"));
	dp->errorDetectionType = errorDetectionStringToType(mConfiguration->getString("error-detection"));
	dp->systematic =         !mConfiguration->getSwitch("non-systematic");
	dp->decoderType =        PolarCode::Decoding::DecoderType::tFlexible;
	dp->codingScheme = -1;

	// Set simulation parameters
	dp->EbN0 =             mConfiguration->getFloat("snr-max");
	dp->BlocksToSimulate = mConfiguration->getLongInt("workload") / dp->N;
	dp->precision =        mConfiguration->getInt("precision");
	dp->amplification =    mConfiguration->getFloat("amplification");
	dp->bitsPerSymbol =    1;

	// Statistics
	//nothing to configure here, all values were set to zero

	return dp;
}

void Simulator::configureSingleRun() {
	DataPoint* singleConfig = getDefaultDataPoint();
	mJobList.push_back(singleConfig);
}

void Simulator::configureCodeLengthSim() {
	DataPoint* jobTemplate = getDefaultDataPoint();

	unsigned long workload = mConfiguration->getLongInt("workload");
	unsigned nMin = mConfiguration->getInt("n-min");
	unsigned nMax = mConfiguration->getInt("n-max");
	float rate = mConfiguration->getFloat("rate");


	for(unsigned n = nMin; n <= nMax; n *= 2) {
		DataPoint* job = new DataPoint(*jobTemplate);

		job->N = n;
		job->K = n*rate;
		job->BlocksToSimulate = workload / job->N;

		mJobList.push_back(job);
	}

	delete jobTemplate;
}

void Simulator::configureDesignSnrSim() {
	DataPoint* jobTemplate = getDefaultDataPoint();
	float dsnrMin = mConfiguration->getFloat("dsnr-min");
	float dsnrMax = mConfiguration->getFloat("dsnr-max");
	unsigned dsnrCount = mConfiguration->getInt("dsnr-count");
	float scale = (dsnrMax-dsnrMin)/(dsnrCount - 1);

	for(unsigned i = 0; i < dsnrCount; ++i) {
		DataPoint* job = new DataPoint(*jobTemplate);

		job->designSNR = dsnrMin + i*scale;

		mJobList.push_back(job);
	}

	delete jobTemplate;
}

void Simulator::configureListLengthSim() {
	DataPoint* jobTemplate = getDefaultDataPoint();
	unsigned lMin = mConfiguration->getInt("l-min");
	unsigned lMax = mConfiguration->getInt("l-max");

	for(unsigned l = lMin; l <= lMax; l *= 2) {
		DataPoint* job = new DataPoint(*jobTemplate);

		job->L = l;

		mJobList.push_back(job);
	}

	delete jobTemplate;
}

void Simulator::configureRateSim() {
	DataPoint* jobTemplate = getDefaultDataPoint();
	float rMin = mConfiguration->getFloat("r-min");
	float rMax = mConfiguration->getFloat("r-max");
	unsigned rCount = mConfiguration->getInt("r-count");
	float scale = (rMax-rMin)/(rCount - 1);

	for(unsigned i = 0; i < rCount; ++i) {
		DataPoint* job = new DataPoint(*jobTemplate);

		job->K = job->N * (rMin + i*scale);//Set K according to rate

		// Round up to nearest byte
		job->K = (job->K+7)/8;
		job->K *= 8;

		mJobList.push_back(job);
	}

	delete jobTemplate;
}

void Simulator::configureAmplificationSim() {
	DataPoint* jobTemplate = getDefaultDataPoint();
	float ampMin = mConfiguration->getFloat("amp-min");
	float ampMax = mConfiguration->getFloat("amp-max");
	unsigned ampCount = mConfiguration->getInt("amp-count");
	float scale = (ampMax-ampMin)/(ampCount - 1);

	for(unsigned i = 0; i < ampCount; ++i) {
		DataPoint* job = new DataPoint(*jobTemplate);

		job->amplification = ampMin + i*scale;

		mJobList.push_back(job);
	}

	delete jobTemplate;
}

void Simulator::configureFixedSim() {
/*	using scheme_t = PolarCode::Decoding::CodingScheme;
	DataPoint* jobTemplate = getDefaultDataPoint();

	std::vector<scheme_t> &registry = PolarCode::Decoding::codeRegistry;

	for(unsigned i = 0; i < registry.size(); ++i) {
		DataPoint* job = new DataPoint(*jobTemplate);

		scheme_t &scheme = registry[i];

		job->N = scheme.blockLength;
		job->K = scheme.infoLength;
		job->designSNR = scheme.designSnr;
		job->systematic = scheme.systematic;
		job->decoderType = PolarCode::Decoding::DecoderType::tFixed;
		job->codingScheme = i;
		job->BlocksToSimulate = mConfiguration->getLongInt("workload") / job->N;

		mJobList.push_back(job);
	}

	delete jobTemplate;*/


	DataPoint* config = getDefaultDataPoint();
	config->decoderType = PolarCode::Decoding::DecoderType::tFixed;
	mJobList.push_back(config);
}

void Simulator::configureDepthFirstSim() {
	DataPoint* jobTemplate = getDefaultDataPoint();
	unsigned lMin = mConfiguration->getInt("l-min");
	unsigned lMax = mConfiguration->getInt("l-max");

//	collectNodeStatistics();

	for(unsigned l = lMin; l <= lMax; l *= 2) {
		DataPoint* job = new DataPoint(*jobTemplate);

		job->L = l;
		job->decoderType = PolarCode::Decoding::DecoderType::tDepthFirst;

		mJobList.push_back(job);
	}

	delete jobTemplate;
}

void Simulator::configureScanSim(bool fastSimplified) {
	DataPoint* jobTemplate = getDefaultDataPoint();
	unsigned lMin = mConfiguration->getInt("l-min");
	unsigned lMax = mConfiguration->getInt("l-max");

	for(unsigned l = lMin; l <= lMax; l *= 2) {
		DataPoint* job = new DataPoint(*jobTemplate);

		job->L = l;
		job->decoderType = fastSimplified ? PolarCode::Decoding::DecoderType::tFastSscan
										  : PolarCode::Decoding::DecoderType::tScan;

		mJobList.push_back(job);
	}

	delete jobTemplate;
}

void Simulator::configureAskSim() {
	DataPoint* jobTemplate = getDefaultDataPoint();
	unsigned bMin = 2;
	unsigned bMax = 10;

	for(unsigned b = bMin; b <= bMax; b++) {
		DataPoint* job = new DataPoint(*jobTemplate);

		job->bitsPerSymbol = b;

		mJobList.push_back(job);
	}

	delete jobTemplate;
}

void Simulator::snrInflateJobList() {
	std::vector<DataPoint*> compactList;
	compactList.clear();
	std::swap(compactList, mJobList);

	float snrMin = mConfiguration->getFloat("snr-min");
	float snrMax = mConfiguration->getFloat("snr-max");
	unsigned snrCount = mConfiguration->getInt("snr-count");
	float scale = (snrMax-snrMin)/(snrCount-1);

	for(DataPoint* job : compactList) {
		for(unsigned i=0; i<snrCount; ++i) {
			// use copy constructor! Otherwise bad side effects are imminent!
			DataPoint* newJob = new DataPoint(*job);
			newJob->EbN0 = snrMin + i*scale;
			if(newJob->precision == 32) {
				//For float-only decoding: Correct LLR-coefficient is
				// L_ch = 4 * abs(alpha)^2 * E_S/N_0
				//and LLR(y|x) = L_ch*y', with y' = y / (abs(alpha) * sqrt(E_S * T_S))
				//
				//Direct source: See Channel Coding II lecture, held by Dr.-Ing. Dirk Wübben, Universität Bremen
				newJob->amplification = 4 * pow(10.0, newJob->EbN0/10.0);//assume abs(alpha)=1
			}
			mJobList.push_back(newJob);
		}
		delete job;
	}
}

void Simulator::configureComparisonSim() {
	std::vector<DataPoint*> jobList;
	DataPoint* jobTemplate = getDefaultDataPoint();
	DataPoint* job;

	jobTemplate->EbN0 = 2.0;
	float ampFloat = 4 * pow(10.0, jobTemplate->EbN0/10.0);

	job = new DataPoint(*jobTemplate);
	job->name = "Fast-SSC32";
	job->decoderType = PolarCode::Decoding::DecoderType::tFlexible;
	job->precision = 32;
	job->amplification = ampFloat;
	job->L = 1;
	jobList.push_back(job);

	job = new DataPoint(*jobTemplate);
	job->name = "Fast-SSC8";
	job->decoderType = PolarCode::Decoding::DecoderType::tFlexible;
	job->precision = 8;
	job->L = 1;
	jobList.push_back(job);

	job = new DataPoint(*jobTemplate);
	job->name = "SCL32";
	job->decoderType = PolarCode::Decoding::DecoderType::tFlexible;
	job->precision = 32;
	job->amplification = ampFloat;
	jobList.push_back(job);

	job = new DataPoint(*jobTemplate);
	job->name = "SCL8";
	job->decoderType = PolarCode::Decoding::DecoderType::tFlexible;
	job->precision = 8;
	jobList.push_back(job);

	job = new DataPoint(*jobTemplate);
	job->name = "SCFlip";
	job->decoderType = PolarCode::Decoding::DecoderType::tDepthFirst;
	job->amplification = ampFloat;
	jobList.push_back(job);

/*	job = new DataPoint(*jobTemplate);
	job->name = "SCAN";
	job->decoderType = PolarCode::Decoding::DecoderType::tScan;
	job->amplification = ampFloat;
	jobList.push_back(job);*/

	job = new DataPoint(*jobTemplate);
	job->name = "Fast-SSCAN";
	job->decoderType = PolarCode::Decoding::DecoderType::tFastSscan;
	job->amplification = ampFloat;
	jobList.push_back(job);


	{// Push two versions of the above list
		auto highRateTemplate = getDefaultDataPoint();

		highRateTemplate->EbN0 = 3.0;
		ampFloat = 4 * pow(10.0, highRateTemplate->EbN0/10.0);
		highRateTemplate->N = 4096;
		highRateTemplate->K = 2048+1024;

		for(auto ljob : jobList) {
			mJobList.push_back(ljob);
			job = new DataPoint(*ljob);
			job->N = highRateTemplate->N;
			job->K = highRateTemplate->K;
			job->EbN0 = highRateTemplate->EbN0;
			if(job->precision == 32) {
				job->amplification = ampFloat;
			}
			mJobList.push_back(job);
		}

		delete highRateTemplate;
	}

	// And add the fixed/templatized decoder
	job = new DataPoint(*jobTemplate);
	job->name = "FFSSC";
	job->decoderType = PolarCode::Decoding::DecoderType::tFixed;
	job->amplification = ampFloat;
	mJobList.push_back(job);

	delete jobTemplate;
}

void Simulator::printCode() {
	DataPoint* config = getDefaultDataPoint();
	auto constructor = new PolarCode::Construction::Bhattacharrya(
						config->N,
						config->K,
						config->designSNR);
	auto frozenBits = constructor->construct();

	unsigned counter = 0;
	for(unsigned i = 0; i < (unsigned)config->N; ++i) {
		if(frozenBits[counter] == i) {
			++counter;
			std::cout << "1,";
		} else {
			std::cout << "0,";
		}
	}
	std::cout << std::endl << std::endl;

	for(auto i : frozenBits) {
		std::cout << i << ",";
	}

	std::cout << std::endl;
}


void Simulator::saveResults() {
	std::string fileName = mConfiguration->getString("output");
	fileName += "_";
	fileName += mConfiguration->getString("simtype");
	fileName += ".csv";
	std::ofstream file(fileName);


	file << "\"N\",\"K\",\"dSNR\",\"C\",\"L\",\"Eb/N0\",\"BPS\",\"BLER\",\"BER\",\"RER\",\"Runs\",\"Errors\",\"Time\",\"Blockspeed\",\"Coded Bitrate\",\"Payload Bitrate\",\"Effective Payload Bitrate\",\"Encoder Bitrate\",\"Amplification\",\"time min\",\"time max\",\"time mean\",\"time deviation\"" << std::endl;

	for(auto job : mJobList) {
	std::vector<float> timeValues = job->timeStat.valueList();
		file<< job->N << ','
			<< job->K << ','
			<< job->designSNR << ','
			<< job->errorDetection << ','
			<< job->L << ','
			<< job->EbN0 << ','
			<< job->bitsPerSymbol << ',';
		if(job->BLER > 0.0)         file << job->BLER << ',';   else file << "1e-99,";
		if(job->BER  > 0.0)         file << job->BER  << ',';   else file << "1e-99,";
		if(job->RER  > 0.0)         file << job->RER  << ',';   else file << "1e-99,";
		file << job->runs << ','
			 << job->errors << ','
			 << job->time.sum << ','
			 << job->blps << ','
			 << job->cbps << ','
			 << job->pbps << ','
			 << job->effectiveRate << ','
			 << job->ebps << ','
			 << job->amplification << ','
			 << int(job->time.min * 1e9) << ','
			 << int(job->time.max * 1e9) << ','
			 << int(job->time.mean * 1e9) << ','
			 << int(job->time.dev * 1e9);
//      for(auto& tp : timeValues){
//        file << ',' << int(tp * 1e9);
//      }
	file << std::endl;
	}
	file.close();
}

void Simulator::saveComparisonResults() {
	std::string fileName = mConfiguration->getString("output");
	fileName += "_";
	fileName += mConfiguration->getString("simtype");
	fileName += ".csv";
	std::ofstream file(fileName);


	file << "\"Name\",\"N\",\"K\",\"dSNR\",\"C\",\"L\",\"Eb/N0\",\"BPS\",\"BLER\",\"BER\",\"RER\",\"Runs\",\"Errors\",\"Time\",\"Blockspeed\",\"Coded Bitrate\",\"Payload Bitrate\",\"Effective Payload Bitrate\",\"Encoder Bitrate\",\"Amplification\",\"time min\",\"time max\",\"time mean\",\"time deviation\"" << std::endl;

	for(auto job : mJobList) {
		file
			<< '"' << job->name << "\","
			<< job->N << ','
			<< job->K << ','
			<< job->designSNR << ','
			<< job->errorDetection << ','
			<< job->L << ','
			<< job->EbN0 << ','
			<< job->bitsPerSymbol << ',';
		if(job->BLER > 0.0)         file << job->BLER << ',';   else file << "1e-99,";
		if(job->BER  > 0.0)         file << job->BER  << ',';   else file << "1e-99,";
		if(job->RER  > 0.0)         file << job->RER  << ',';   else file << "1e-99,";
		file << job->runs << ','
			 << job->errors << ','
			 << job->time.sum << ','
			 << job->blps << ','
			 << job->cbps << ','
			 << job->pbps << ','
			 << job->effectiveRate << ','
			 << job->ebps << ','
			 << job->amplification << ','
			 << int(job->time.min * 1e9) << ','
			 << int(job->time.max * 1e9) << ','
			 << int(job->time.mean * 1e9) << ','
			 << int(job->time.dev * 1e9);
	file << std::endl;
	}
	file.close();
}

void SimThread(Simulator* Sim, int workerId) {
	SimulationWorker *worker = new SimulationWorker(Sim, workerId);
	worker->run();
	delete worker;
}

SimulationWorker::SimulationWorker(Simulator* Sim, int workerId)
	: mSim(Sim)
	, mModulator(new SignalProcessing::Modulation::Bpsk())
	, mDemodulator(new SignalProcessing::Modulation::Bpsk())
	, mTransmitter(new SignalProcessing::Transmission::Awgn())
	, mAmplifier(new SignalProcessing::Transmission::Scale())
	, mWorkerId(workerId)
{}

SimulationWorker::~SimulationWorker() {
	delete mTransmitter;
	delete mModulator;
	delete mDemodulator;
	delete mAmplifier;
}

void SimulationWorker::run() {
	while((mJob = mSim->getJob()) != nullptr) {
		jobStartingOutput();
		selectFrozenBits();
		setCoders();
		setErrorDetector();
		setChannel();
		allocateMemory();

		unsigned long blocksToSimulate = mJob->BlocksToSimulate;
		unsigned long warmUpBlocks = std::min(blocksToSimulate / 8, 1000UL);

		//Warmup
		warmup = true;
		for(unsigned block = 0; block < warmUpBlocks; ++block) {
			generateData();
			encode();
			modulate();
			transmit();
			demodulate();
			decode();
			countErrors();
		}

		//Actual simulation
		warmup = false;
		for(unsigned block = 0; block < blocksToSimulate; ++block) {
			generateData();
			encode();
			modulate();
			transmit();
			demodulate();
			decode();
			countErrors();
		}
		calculateStatistics();
		cleanup();
		jobEndingOutput();
	}
}

void SimulationWorker::startTiming() {
	mTimeStart = std::chrono::high_resolution_clock::now();
}

void SimulationWorker::stopTiming() {
	using namespace std::chrono;
	mTimeEnd = high_resolution_clock::now();
	mTimeUsed = duration_cast<duration<float>>(mTimeEnd - mTimeStart);
}

constexpr std::array<int, 1024> fixed1024FrozenSet = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,
	1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,0,1,0,0,0,1,1,1,1,1,1,1,0,1,1,1,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,0,0,0,0,1,1,
	1,1,1,1,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,0,0,0,
	1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,
	1,0,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,0,1,0,0,0,0,0,0,0,
	1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,
	1,1,1,1,1,1,1,0,0,0,1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,1,1,1,0,1,0,0,0,1,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,1,1,1,0,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,
	0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const std::vector<unsigned> fixed1024FrozenIdx = {
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
	28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,
	53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,
	78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,
	102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,
	121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,
	140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,
	159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,
	178,179,180,181,182,183,184,185,186,187,188,189,190,192,193,194,195,196,197,
	198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,
	217,218,219,220,221,222,224,225,226,227,228,229,230,231,232,233,234,235,236,
	240,241,242,244,248,256,257,258,259,260,261,262,263,264,265,266,267,268,269,
	270,271,272,273,274,275,276,277,278,279,280,281,282,283,284,285,286,287,288,
	289,290,291,292,293,294,295,296,297,298,299,300,301,302,303,304,305,306,307,
	308,309,310,311,312,313,314,315,316,317,320,321,322,323,324,325,326,327,328,
	329,330,331,332,333,334,335,336,337,338,339,340,341,342,343,344,345,346,348,
	352,353,354,355,356,357,358,360,361,362,364,368,369,384,385,386,387,388,389,
	390,391,392,393,394,395,396,397,398,400,401,402,403,404,405,406,408,409,410,
	416,417,418,419,420,421,424,432,448,449,450,452,456,464,480,512,513,514,515,
	516,517,518,519,520,521,522,523,524,525,526,527,528,529,530,531,532,533,534,
	535,536,537,538,539,540,541,542,543,544,545,546,547,548,549,550,551,552,553,
	554,555,556,557,558,560,561,562,563,564,565,566,568,569,570,572,576,577,578,
	579,580,581,582,583,584,585,586,587,588,589,590,592,593,594,595,596,597,600,
	608,609,610,612,616,624,640,641,642,643,644,645,646,647,648,649,650,651,652,
	656,657,658,660,664,672,673,674,676,680,704,705,706,768,769,770,771,772,773,
	774,776,777,778,780,784,785,786,800,832,896};

void SimulationWorker::selectFrozenBits() {
	if(mJob->decoderType != PolarCode::Decoding::DecoderType::tFixed) {
		mConstructor = new PolarCode::Construction::Bhattacharrya(mJob->N, mJob->K, mJob->designSNR);
		mFrozenBits = mConstructor->construct();
	} else {
		mConstructor = nullptr;
//		std::vector<PolarCode::Decoding::CodingScheme> &registry = PolarCode::Decoding::codeRegistry;
//		std::vector<unsigned> &frozenBits = registry[mJob->codingScheme].frozenBits;
//		mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
		mFrozenBits.assign(fixed1024FrozenIdx.begin(), fixed1024FrozenIdx.end());
	}
}


void SimulationWorker::setCoders() {
	mEncoder = new PolarCode::Encoding::ButterflyFipPacked(mJob->N, mFrozenBits);

	if(mJob->decoderType == PolarCode::Decoding::DecoderType::tFixed) {
		//mDecoder = new PolarCode::Decoding::FixedChar(mJob->codingScheme);
		mDecoder = new PolarCode::Decoding::TemplatizedFloat<1024, fixed1024FrozenSet>(fixed1024FrozenIdx);
	} else if(mJob->decoderType == PolarCode::Decoding::DecoderType::tDepthFirst) {
		mDecoder = new PolarCode::Decoding::DepthFirst(mJob->N, mJob->L, mFrozenBits);
	} else if(mJob->decoderType == PolarCode::Decoding::DecoderType::tScan) {
		mDecoder = new PolarCode::Decoding::Scan(mJob->N, mJob->L, mFrozenBits);
	} else if(mJob->decoderType == PolarCode::Decoding::DecoderType::tFastSscan) {
		mDecoder = new PolarCode::Decoding::FastSscanFloat(mJob->N, mJob->L, mFrozenBits);
	} else {
		if(mJob->L > 1) {
			switch(mJob->precision) {
			case 8:
				mDecoder = new PolarCode::Decoding::AdaptiveChar(mJob->N, mJob->L, mFrozenBits);
				break;
			case 32:
				mDecoder = new PolarCode::Decoding::AdaptiveFloat(mJob->N, mJob->L, mFrozenBits);
				//mDecoder = new PolarCode::Decoding::SclAvxFloat(mJob->N, mJob->L, mFrozenBits);
				break;
			case 832:
				mDecoder = new PolarCode::Decoding::AdaptiveMixed(mJob->N, mJob->L, mFrozenBits);
				break;
			default:
				std::cerr << "No decoder present for " << mJob->precision << "-bit decoding." << std::endl;
				exit(1);
			}
		} else {
			switch(mJob->precision) {
			case 8:
			case 832:
				mDecoder = new PolarCode::Decoding::FastSscFipChar(mJob->N, mFrozenBits);
				break;
			case 32:
				mDecoder = new PolarCode::Decoding::FastSscAvxFloat(mJob->N, mFrozenBits);
				//mDecoder = new PolarCode::Decoding::FastSscanFloat(mJob->N, mFrozenBits);
				break;
			default:
				std::cerr << "No decoder present for " << mJob->precision << "-bit decoding." << std::endl;
				exit(1);
			}
		}
	}

	mEncoder->setSystematic(mJob->systematic);
	mDecoder->setSystematic(mJob->systematic);
}

void SimulationWorker::setErrorDetector() {
	if(mJob->errorDetection >= mJob->K) {
		mJob->errorDetection = 0;
		mJob->errorDetectionType = "none";
	}
	if(mJob->errorDetectionType == "crc") {
		switch(mJob->errorDetection) {
			case 8:
				mErrorDetector = new PolarCode::ErrorDetection::CRC8();
				break;
			case 32:
				mErrorDetector = new PolarCode::ErrorDetection::CRC32();
				break;
			default:
				mErrorDetector = new PolarCode::ErrorDetection::Dummy();
		}
	} else {
		mErrorDetector = new PolarCode::ErrorDetection::Dummy();
	}

	mEncoder->setErrorDetection(mErrorDetector);
	mDecoder->setErrorDetection(mErrorDetector);
}

void SimulationWorker::setChannel() {
	if(mJob->bitsPerSymbol > 1) {
		delete mModulator;
		delete mDemodulator;
		mModulator = new SignalProcessing::Modulation::Ask(mJob->bitsPerSymbol);
		mDemodulator = new SignalProcessing::Modulation::Ask(mJob->bitsPerSymbol);
	}
	// Set channel SNR to energy per bit over noise energy for
	// AWGN channels.
	// Source: Chapter 11.4. in Nachrichtenübertragung by K.-D. Kammeyer, 2011
	float EbN0_linear = pow(10.0, mJob->EbN0 / 10.0);
	// Adapt SNR to bit count per symbol and code rate to get
	// true energy per information bit ("*2.0" to emulate complex signals)
	float EsN0_linear = EbN0_linear * mJob->bitsPerSymbol/* * 2.0*/;
	EsN0_linear *= mJob->K;
	EsN0_linear /= mJob->N;
	mTransmitter->setEsN0Linear(EsN0_linear);

	mAmplifier->setFactor(mJob->amplification);
}

void SimulationWorker::allocateMemory() {
	mInputData = new unsigned char[mJob->K / 8];
	mEncodedData = new PolarCode::PackedContainer(mJob->N);
	mDecodedData = nullptr;
}

void SimulationWorker::generateData() {
	SignalProcessing::Random::Generator generator;
	unsigned long* lData = reinterpret_cast<unsigned long*>(mInputData);
	unsigned nBits = mJob->K - mJob->errorDetection;
	unsigned nLongs = nBits / 64;
	unsigned nBytes = (nBits - nLongs*64) / 8;


	for(unsigned i=0; i<nLongs; ++i) {
		generator.get64(lData + i);
	}
	if(nBytes) {
		unsigned long rem;
		generator.get64(&rem);
		memcpy(lData + nLongs, &rem, nBytes);
	}
}

void SimulationWorker::encode() {
	startTiming();
	mEncoder->setInformation(mInputData);
	mEncoder->encode();
	mEncoder->getEncodedData(mEncodedData->data());
	stopTiming();
	if(!warmup)
		mJob->encTime += mTimeUsed.count();
}

void SimulationWorker::modulate() {
	mModulator->setInputData(mEncodedData);
	mModulator->modulate();
	mSignal = mModulator->outputSignal();
}

void SimulationWorker::transmit() {
/*	float sum = 0.0f;
	for(float f : *mSignal) {
		sum += f * f;
	}
	sum /= mSignal->size();
	std::cout << "Pre-transmit energy: " << sum << std::endl
			  << "Expected energy: " << 1.0f << std::endl
			  << std::endl;
*/
	mTransmitter->setSignal(mSignal);
	mTransmitter->transmit();
/*
	sum = 0.0f;
	for(float f : *mSignal) {
		sum += f * f;
	}
	sum /= mSignal->size();
	std::cout << "Post-transmit energy: " << sum << std::endl
			  << "Expected energy: " << (1.0f + 0.5f / mTransmitter->EsNoLin()) << std::endl
			  << std::endl;
*/
}

void SimulationWorker::demodulate() {
	mDemodulator->setInputSignal(mSignal);
	mDemodulator->demodulate();
	mSignal = mDemodulator->outputSignal();
}

void SimulationWorker::decode() {
	bool success;

	mAmplifier->setSignal(mSignal);
	mAmplifier->transmit();

	startTiming();
	mDecoder->setSignal(mSignal->data());
	success = mDecoder->decode();
	mDecodedData = mDecoder->packedOutput();
	stopTiming();

	if(!success && !warmup) mJob->reportedErrors++;
	if(!warmup)	mJob->timeStat.insert(mTimeUsed.count());
}

void SimulationWorker::countErrors() {
	unsigned long biterrors = 0;
	unsigned long long* liData = reinterpret_cast<unsigned long long*>(mInputData);
	unsigned long long* loData = reinterpret_cast<unsigned long long*>(mDecodedData);
	unsigned nBits = mJob->K;
	unsigned nLongs = nBits / 64;
	unsigned nBytes = (nBits - nLongs*64) / 8;


	for(unsigned i=0; i<nLongs; ++i) {
		biterrors += _mm_popcnt_u64(liData[i] ^ loData[i]);
	}
	if(nBytes) {
		unsigned long remIn = 0, remOut = 0;
		memcpy(&remIn, liData+nLongs, nBytes);
		memcpy(&remOut, loData+nLongs, nBytes);
		biterrors += _mm_popcnt_u64(remIn ^ remOut);
	}
	if(!warmup) {
		mJob->biterrors += biterrors;
		if(biterrors) {
			mJob->errors++;
		}
		mJob->runs++;
	}
}

void SimulationWorker::calculateStatistics() {
	mJob->time = mJob->timeStat.evaluate();
	//mJob->timeStat.printContents();
	mJob->bits = mJob->runs * (mJob->K - mJob->errorDetection);
	mJob->BLER = (float)mJob->errors / mJob->runs;
	mJob->BER = (double)mJob->biterrors / ((double)mJob->runs * (double)mJob->K);
	mJob->RER = (float)mJob->reportedErrors / mJob->runs;
	mJob->blps = mJob->runs;
	mJob->cbps = mJob->runs * mJob->N;
	mJob->pbps = mJob->bits;
	mJob->ebps = mJob->cbps;
	mJob->blps /= mJob->time.sum;
	mJob->cbps /= mJob->time.sum;
	mJob->pbps /= mJob->time.sum;
	mJob->ebps /= mJob->encTime;
	mJob->effectiveRate = (mJob->runs-mJob->errors+0.0f)
						* (mJob->K - mJob->errorDetection)
						/ mJob->time.sum;

}

void SimulationWorker::jobStartingOutput() {
	std::string output;
	output.clear();

	output += "[";
	output += std::to_string(mWorkerId);
	output += "] N=" + std::to_string(mJob->N);
	output += ", K=" + std::to_string(mJob->K);
	output += ", L=" + std::to_string(mJob->L);
	output += ", dSNR=" + std::to_string(mJob->designSNR);
  output += ", ErrorDetector=" + mJob->errorDetectionType + std::to_string(mJob->errorDetection);
	output += ", SNR=" + std::to_string(mJob->EbN0);
	output += "\n";

	std::cout << output;
}

void SimulationWorker::jobEndingOutput() {
	std::string output;
	output.clear();

	output += "[";
	output += std::to_string(mWorkerId);
	output += "] BLER=" + std::to_string(mJob->BLER);
	output += ", BER=" + std::to_string(mJob->BER);
	output += ", RER=" + std::to_string(mJob->RER);
	output += ", throughput:" + std::to_string(mJob->cbps * 1e-6);
	output += "Mbps, delay[µs]=[" + std::to_string(mJob->time.min * 1e6);
	output += ";" + std::to_string(mJob->time.max * 1e6);
	output += "](" + std::to_string(mJob->time.mean * 1e6);
	output += "," + std::to_string(mJob->time.dev * 1e6);
	output += ") [min;max](mean,dev)\n";

	std::cout << output;

}

void SimulationWorker::cleanup() {
	delete [] mInputData;
	delete mEncodedData;

	delete mDecoder;
	delete mEncoder;
	delete mErrorDetector;
	if(mConstructor) delete mConstructor;
}

}//namespace Simulation
