#ifndef PCSIM_SIMULATOR_H
#define PCSIM_SIMULATOR_H

#include <atomic>
#include <chrono>
#include <queue>

#include <polarcode/construction/bhattacharrya.h>
#include <polarcode/encoding/butterfly_avx2_packed.h>
#include <polarcode/decoding/fastssc_avx_float.h>
#include <polarcode/decoding/fastssc_avx2_char.h>
#include <polarcode/decoding/scl_avx2_char.h>
#include <polarcode/errordetection/dummy.h>
#include <polarcode/errordetection/crc8.h>
#include <polarcode/errordetection/crc32.h>
#include <polarcode/errordetection/errordetector.h>

#include <signalprocessing/random.h>
#include <signalprocessing/modulation/bpsk.h>
#include <signalprocessing/transmission/awgn.h>

#include "setup.h"

namespace Simulation {

inline int errorDetectionStringToId(std::string str) {
	if(str == "crc8") {
		return 8;
	} else if(str == "crc32") {
		return 32;
	} else {
		//implicit "none"
		return 0;
	}
}

struct DataPoint
{
	//Codec-Parameters
	float designSNR; int N,K,L;
	int errorDetection;//0=none,8/32=crc
	bool systematic;

	//Simulation-Parameters
	float EbN0;
	long BlocksToSimulate;

	//Statistics
	int runs, bits, errors, reportedErrors, biterrors;
	float BLER, BER, RER;
	float time;//in seconds
	float blps,cbps,pbps;//blocks/coded bits/payload bits per second
	float effectiveRate;
	float encTime, ebps;//encoding statistics
};

/*!
 * \brief The Simulator class
 */
class Simulator {
	Setup::Configurator *mConfiguration;

	std::vector<DataPoint*> mJobList;
	std::atomic<unsigned> mNextJob;

	DataPoint* getDefaultDataPoint();
	void configureSingleRun();
	void configureCodeLengthSim();
	void configureDesignSnrSim();
	void configureListLengthSim();
	void configureRateSim();
	void snrInflateJobList();

	void saveResults();

public:
	/*!
	 * \brief Create a simulator object and configure it.
	 */
	Simulator(Setup::Configurator*);
	~Simulator();

	/*!
	 * \brief Run the simulation.
	 */
	void run();

	/*!
	 * \brief Worker threads can poll for jobs by getJob().
	 *
	 * This function returns a pointer to a DataPoint structure, which contains
	 * simulation parameters and return values.
	 *
	 * \return Pointer to a previously unassigned job or nullptr, if all work is done.
	 */
	DataPoint* getJob();
};

/*!
 * SimThread is a function which pulls and executes a row of jobs from
 * the given Simulator object until Simulator::getJob() returns nullptr.
 */
void SimThread(Simulator*, int workerId);

class SimulationWorker {
	Simulator *mSim;
	DataPoint *mJob;
	PolarCode::Construction::Bhattacharrya *mConstructor;
	PolarCode::Encoding::Encoder *mEncoder;
	PolarCode::Decoding::Decoder *mDecoder;
	PolarCode::ErrorDetection::Detector *mErrorDetector;

	SignalProcessing::Modulation::Modem *mModem;
	SignalProcessing::Transmission::Awgn *mTransmitter;

	std::vector<unsigned> mFrozenBits;

	unsigned char* mInputData;
	PolarCode::PackedContainer *mEncodedData;
	std::vector<float> *mSignal;
	unsigned char* mDecodedData;

	std::chrono::high_resolution_clock::time_point mTimeStart, mTimeEnd;
	std::chrono::duration<float> mTimeUsed;

	int mWorkerId;

	void startTiming();
	void stopTiming();

	void selectFrozenBits();
	void setCoders();
	void setErrorDetector();
	void setChannel();
	void allocateMemory();

	void generateData();
	void calculateChecksum();
	void encode();
	void modulate();
	void transmit();
	void decode();
	void countErrors();

	void calculateStatistics();

	void jobStartingOutput();
	void jobEndingOutput();

	void cleanup();


public:
	SimulationWorker(Simulator* Sim, int workerId);
	~SimulationWorker();

	void run();
};

}

#endif //PCSIM_SIMULATOR_H
