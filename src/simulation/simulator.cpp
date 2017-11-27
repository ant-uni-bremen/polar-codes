#include "simulator.h"

#include <thread>
#include <fstream>
#include <iostream>
#include <cmath>

#include <polarcode/encoding/butterfly_avx2_packed.h>

#include <polarcode/decoding/fastssc_avx_float.h>
#include <polarcode/decoding/fastssc_avx2_char.h>
#include <polarcode/decoding/scl_avx2_char.h>
#include <polarcode/decoding/scl_avx_float.h>
#include <polarcode/decoding/adaptive_float.h>
#include <polarcode/decoding/adaptive_char.h>
#include <polarcode/decoding/adaptive_mixed.h>

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
	saveResults();
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
	dp->designSNR =        mConfiguration->getFloat("design-snr");
	dp->N =                mConfiguration->getInt("blocklength");
	dp->K = dp->N *        mConfiguration->getFloat("rate");
	dp->L =                mConfiguration->getInt("pathlimit");
	dp->errorDetection =   errorDetectionStringToId(mConfiguration->getString("error-detection"));
	dp->systematic =      !mConfiguration->getSwitch("non-systematic");

	// Set simulation parameters
	dp->EbN0 =             mConfiguration->getFloat("snr-max");
	dp->BlocksToSimulate = mConfiguration->getLongInt("workload") / dp->N;
	dp->precision =        mConfiguration->getInt("precision");
	dp->amplification =    mConfiguration->getFloat("amplification");

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
			DataPoint* newJob = new DataPoint;
			memcpy(newJob, job, sizeof(DataPoint));
			newJob->EbN0 = snrMin + i*scale;
			if(newJob->precision == 32) {
				//For float-only decoding: Correct LLR-coefficient is
				// L_ch = 4 * abs(alpha)^2 * E_S/N_0
				//and LLR(y|x) = L_ch*y', with y' = y / (abs(alpha) * sqrt(E_S * T_S))
				//
				//Direct source: See Channel Coding II lecture, held by Dr.-Ing. Dirk Wübben, Universität Bremen
				newJob->amplification = 4 * newJob->EbN0;//assume abs(alpha)=1
			}
			mJobList.push_back(newJob);
		}
		delete job;
	}
}

void Simulator::saveResults() {
	std::string fileName = mConfiguration->getString("output");
	fileName += "_";
	fileName += mConfiguration->getString("simtype");
	fileName += ".csv";
	std::ofstream file(fileName);

	file << "\"N\",\"K\",\"dSNR\",\"C\",\"L\",\"Eb/N0\",\"BLER\",\"BER\",\"RER\",\"Runs\",\"Errors\",\"Time\",\"Blockspeed\",\"Coded Bitrate\",\"Payload Bitrate\",\"Effective Payload Bitrate\",\"Encoder Bitrate\",\"Amplification\",\"time min\",\"time max\",\"time mean\",\"time deviation\"" << std::endl;

	for(auto job : mJobList) {
		file<< job->N << ','
			<< job->K << ','
			<< job->designSNR << ','
			<< job->errorDetection << ','
			<< job->L << ','
			<< job->EbN0 << ',';
		if(job->BLER > 0.0)         file << job->BLER << ',';   else file << "nan,";
		if(job->BER  > 0.0)         file << job->BER  << ',';   else file << "nan,";
		if(job->RER  > 0.0)         file << job->RER  << ',';   else file << "nan,";
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
			 << int(job->time.dev * 1e9) << ','
			 << std::endl;
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
	, mModem(new SignalProcessing::Modulation::Bpsk())
	, mTransmitter(new SignalProcessing::Transmission::Awgn())
	, mAmplifier(new SignalProcessing::Transmission::Scale())
	, mWorkerId(workerId)
{}

SimulationWorker::~SimulationWorker() {
	delete mTransmitter;
	delete mModem;
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

		//Warmup
		warmup = true;
		for(unsigned block = 0; block < 10000; ++block) {
			generateData();
			encode();
			modulate();
			transmit();
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

void SimulationWorker::selectFrozenBits() {
	mConstructor = new PolarCode::Construction::Bhattacharrya(mJob->N, mJob->K, mJob->designSNR);
	mFrozenBits = mConstructor->construct();
}

void SimulationWorker::setCoders() {
	mEncoder = new PolarCode::Encoding::ButterflyAvx2Packed(mJob->N, mFrozenBits);

	if(mJob->L > 1) {
		switch(mJob->precision) {
		case 8:
			mDecoder = new PolarCode::Decoding::AdaptiveChar(mJob->N, mJob->L, mFrozenBits);
			break;
		case 32:
			mDecoder = new PolarCode::Decoding::AdaptiveFloat(mJob->N, mJob->L, mFrozenBits);
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
			mDecoder = new PolarCode::Decoding::FastSscAvx2Char(mJob->N, mFrozenBits);
			break;
		case 32:
			mDecoder = new PolarCode::Decoding::FastSscAvxFloat(mJob->N, mFrozenBits);
			break;
		default:
			std::cerr << "No decoder present for " << mJob->precision << "-bit decoding." << std::endl;
			exit(1);
		}
	}
}
void SimulationWorker::setErrorDetector() {
	switch(mJob->errorDetection) {
		case 8: mErrorDetector = new PolarCode::ErrorDetection::CRC8(); break;
		case 32: mErrorDetector = new PolarCode::ErrorDetection::CRC32(); break;
		default: mErrorDetector = new PolarCode::ErrorDetection::Dummy();
	}
	mEncoder->setErrorDetection(mErrorDetector);
	mDecoder->setErrorDetection(mErrorDetector);
}

void SimulationWorker::setChannel() {
	// Set channel SNR to energy per bit over noise energy for
	// real-valued AWGN channels.
	// Source: Chapter 11.4. in Nachrichtenübertragung by K.-D. Kammeyer, 2011
	float EbN0_linear = pow(10.0, mJob->EbN0 / 10.0) * 2.0;
	// Adapt SNR to code rate to get true energy per information bit
	EbN0_linear *= mJob->K;
	EbN0_linear /= mJob->N;
	mTransmitter->setEsN0Linear(EbN0_linear);

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
	mModem->setInputData(mEncodedData);
	mModem->modulate();
	mSignal = mModem->outputSignal();
}

void SimulationWorker::transmit() {
	mTransmitter->setSignal(mSignal);
	mTransmitter->transmit();

	mAmplifier->setSignal(mSignal);
	mAmplifier->transmit();
}

void SimulationWorker::decode() {
	bool success;

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
	mJob->BLER = (float)mJob->errors/mJob->runs;
	mJob->BER = (float)mJob->biterrors/mJob->bits;
	mJob->RER = (float)mJob->reportedErrors/mJob->runs;
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
	output += ", payload:" + std::to_string(mJob->pbps * 1e-6);
	output += "Mbps, delay[µs]=" + std::to_string(mJob->time.min * 1e6);
	output += "/" + std::to_string(mJob->time.max * 1e6);
	output += "/" + std::to_string(mJob->time.mean * 1e6);
	output += "/" + std::to_string(mJob->time.dev * 1e6);
	output += " (min/max/mean/dev)\n";

	std::cout << output;

}

void SimulationWorker::cleanup() {
	delete [] mInputData;
	delete mEncodedData;

	delete mDecoder;
	delete mEncoder;
	delete mErrorDetector;
	delete mConstructor;
}

}//namespace Simulation
