/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "simulator.h"

#include <thread>
#include <fstream>
#include <iostream>
#include <cmath>

#include <signalprocessing/modulation/ask.h>
#include <polarcode/encoding/butterfly_fip_packed.h>
#include <polarcode/decoding/errorlocator.h>

namespace SimulationErrorLocator {

Simulator::Simulator(Setup::Configurator *config)
	: mConfiguration(config) {
	configure();
}

Simulator::~Simulator() {
	delete mJob;
}

void Simulator::run() {
	// Get number of concurrently active simulation threads
	unsigned threadCount = mConfiguration->getInt("threads");
	std::vector<std::thread> Threads;

	// Start the work
	try {
		for(unsigned i = 0; i < threadCount; ++i) {
			Threads.push_back(std::thread(SimThread, this, i + 1));
		}

		// Wait for all threads to stop
		for(auto& Thread : Threads) {
			Thread.join();
		}
	}
	catch(std::system_error& e) {
		std::cout << "Cannot use multithreaded simulation!\n";
		std::cout << "what(): " << e.what() << std::endl;
		SimThread(this, 1);
	}



	// Write results into file
	saveResults();
}

DataPoint* Simulator::getJob() {
	return mJob;
}

void Simulator::configure() {
	mJob = new DataPoint();

	// Set simulation parameters
	mJob->EbN0 =                mConfiguration->getFloat("snr");
	mJob->BlocksToSimulate =    mConfiguration->getLongInt("workload")
							  / mConfiguration->getInt("threads");

	// Set code parameters
	mJob->designSNR =           mConfiguration->getFloat("design-snr");
	unsigned blockLength =
					  mJob->N = mConfiguration->getInt("blocklength");
	mJob->K = mJob->N *         mConfiguration->getFloat("rate");

	// Statistics
	mJob->additionalErrors.resize(blockLength);
	mJob->firstErrorPosition.resize(blockLength);

	for(unsigned bit = 0; bit < blockLength; ++bit) {
		mJob->additionalErrors[bit].clear();
		mJob->firstErrorPosition[bit] = 0;
	}



	{
		PolarCode::Construction::Bhattacharrya constructor(mJob->N, mJob->K, mJob->designSNR);
		std::vector<unsigned> frozen = constructor.construct();
		mJob->frozenSet.resize(blockLength);
		unsigned frozenCounter = 0;
		for(unsigned bit = 0; bit < blockLength; ++bit) {
			if(frozenCounter == frozen.size()) {
				mJob->frozenSet[bit] = false;
			} else if(frozen[frozenCounter] == bit) {
				++frozenCounter;
				mJob->frozenSet[bit] = true;
			} else {
				mJob->frozenSet[bit] = false;
			}
		}
	}
}

void Simulator::saveResults() {
	std::string fileName = mConfiguration->getString("output");
	fileName += "_";
	fileName += std::to_string(mJob->N);
	fileName += "_";
	fileName += std::to_string(mJob->K);
	fileName += "_";
	fileName += std::to_string(mJob->EbN0);
	fileName += ".csv";
	std::ofstream file(fileName);

	file << "\"Index\",\"Frozen\",\"First error histogram\",\"Mean additional\",\"Dev add\",\"Min add\",\"Max add\"" << std::endl;

	for(int bit = 0; bit < mJob->N; ++bit) {
		StatisticsOutput stat = mJob->additionalErrors[bit].evaluate();
		file << bit
			 << "," << (mJob->frozenSet[bit] ? "\"F\"" : "\"I\"")
			 << "," << mJob->firstErrorPosition[bit]
			 << "," << stat.mean
			 << "," << stat.dev
			 << "," << stat.min
			 << "," << stat.max
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
	, mJob(mSim->getJob())
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
	jobStartingOutput();
	selectFrozenBits();
	setCoders();
	setChannel();
	allocateMemory();

	unsigned long blocksToSimulate = mJob->BlocksToSimulate;

	for(unsigned block = 0; block < blocksToSimulate; ++block) {
		generateData();
		encode();
		modulate();
		transmit();
		demodulate();
		decode();
		countErrors();
	}

	cleanup();
	jobEndingOutput();
}



void SimulationWorker::selectFrozenBits() {
	mConstructor = new PolarCode::Construction::Bhattacharrya(mJob->N, mJob->K, mJob->designSNR);
	mFrozenBits = mConstructor->construct();
}

void SimulationWorker::setCoders() {
	mEncoder = new PolarCode::Encoding::ButterflyFipPacked(mJob->N, mFrozenBits);
	mDecoder = new PolarCode::Decoding::ErrorLocator(mJob->N, mFrozenBits);
	mReferenceDecoder = new PolarCode::Decoding::ErrorLocator(mJob->N, mFrozenBits);
	mReferenceDecoder->setAsReferenceDecoder();

	mEncoder->setSystematic(false);
}

void SimulationWorker::setChannel() {
	float EbN0_linear = pow(10.0, mJob->EbN0 / 10.0);
	float EsN0_linear = EbN0_linear;
	EsN0_linear *= mJob->K;
	EsN0_linear /= mJob->N;
	mTransmitter->setEsN0Linear(EsN0_linear);
	mAmplifier->setFactor(4.0 * EsN0_linear);
}

void SimulationWorker::allocateMemory() {
	mInputData = new unsigned char[mJob->K / 8];
	mEncodedData = new PolarCode::PackedContainer(mJob->N);
	mDecodedData = nullptr;
}

void SimulationWorker::generateData() {
	SignalProcessing::Random::Generator generator;
	unsigned long* lData = reinterpret_cast<unsigned long*>(mInputData);
	unsigned nBits = mJob->K;
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
	mEncoder->setInformation(mInputData);
	mEncoder->encode();
	mEncoder->getEncodedData(mEncodedData->data());
}

void SimulationWorker::modulate() {
	mModulator->setInputData(mEncodedData);
	mModulator->modulate();
	mSignal = mModulator->outputSignal();
	mReferenceSignal.assign(mSignal->begin(), mSignal->end());
}

void SimulationWorker::transmit() {
	mTransmitter->setSignal(mSignal);
	mTransmitter->transmit();
}

void SimulationWorker::demodulate() {
	mDemodulator->setInputSignal(mSignal);
	mDemodulator->demodulate();
	mSignal = mDemodulator->outputSignal();
}

void SimulationWorker::decode() {
	mAmplifier->setSignal(mSignal);
	mAmplifier->transmit();

	mReferenceDecoder->setSignal(mReferenceSignal.data());
	mReferenceDecoder->decode();

	mDecoder->setSignal(mSignal->data());
	mDecoder->setDesiredOutput(mReferenceDecoder->getOutput());
	mDecoder->decode();
}

void SimulationWorker::countErrors() {
	std::lock_guard<std::mutex> lock(mJob->mutex);
	mJob->runs++;

	auto corrections = mDecoder->correctionCount();
	auto firstCorrection = mDecoder->firstError();

	if(corrections > 0) {
		mJob->firstErrorPosition[firstCorrection]++;
		mJob->additionalErrors[firstCorrection].insert(corrections);
	}
}

void SimulationWorker::jobStartingOutput() {
	std::string output;
	output.clear();

	output += "[";
	output += std::to_string(mWorkerId);
	output += "] N=" + std::to_string(mJob->N);
	output += ", K=" + std::to_string(mJob->K);
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
	output += "] Finished\n";

	std::cout << output;

}

void SimulationWorker::cleanup() {
	delete [] mInputData;
	delete mEncodedData;

	delete mDecoder;
	delete mEncoder;
	delete mConstructor;
}

}//namespace Simulation
