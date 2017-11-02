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
#include <polarcode/decoding/scl_avx_float.h>
#include <polarcode/decoding/adaptive_float.h>
#include <polarcode/errordetection/dummy.h>
#include <polarcode/errordetection/crc8.h>
#include <polarcode/errordetection/crc32.h>
#include <polarcode/errordetection/errordetector.h>

#include <signalprocessing/random.h>
#include <signalprocessing/modulation/bpsk.h>
#include <signalprocessing/transmission/awgn.h>
#include <signalprocessing/transmission/rayleigh.h>

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

/*!
 * \brief A collection of simulation input/output, called 'job'.
 *
 * This structure holds the parameters for a simulation job, including coding
 * parameters and statistical outputs after finishing the job.
 */
struct DataPoint
{
	//Codec-Parameters
	float designSNR;///< Design-SNR for code construction
	int N;///< Blocklength
	int K;///< Information length
	int L;///< List length for list-decoding
	int errorDetection;///< 0=none,8/32=crc
	bool systematic;///< True, if systematic coding will be used

	//Simulation-Parameters
	float EbN0;///< Bit-energy to noise-energy ratio for AWGN-channel
	long BlocksToSimulate;///< Determines the BLER-precision

	//Statistics
	int runs;///< Actual number of blocks simulated
	int bits;///< Number of payload bits sent out
	int errors;///< ~ erroneous blocks
	int reportedErrors;///< ~ block errors reported by error detection
	int biterrors;///< ~ flipped payload bits
	float apparentlyBestMetric;///< Lowest metric of output candidates (before error detection)
	float selectedPathMetric;///< Metric of the selected decoding path (after error detection)

	float BLER;///< Block Error Rate
	float BER;///< Bit Error Rate
	float RER;///< Reported Error Rate
	float time;///< Decoding time in seconds
	float blps;///< Blocks per second
	float cbps;///< Channel bits per second
	float pbps;///< Payload bits per second (including errors)
	float effectiveRate;///< Successfully transmitted payload bits per second
	float encTime;///< Encoding time in seconds
	float ebps;///< Encoder speed in bits per second
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

/*!
 * \brief The SimulationWorker class
 *
 * This class contains all static memory for a simulation environment.
 * An object of this class is generated per thread.
 */
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
	/*!
	 * \brief Initialize the worker
	 * \param Sim Pointer to a Simulator object, which provides jobs
	 * \param workerId The ID of this worker, for information printing
	 */
	SimulationWorker(Simulator* Sim, int workerId);
	~SimulationWorker();

	/*!
	 * \brief After creating the job list, execute all of them via run().
	 */
	void run();
};

}

#endif //PCSIM_SIMULATOR_H
