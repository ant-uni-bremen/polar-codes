/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PCSIM_SIMULATOR_H
#define PCSIM_SIMULATOR_H

#include <atomic>
#include <chrono>
#include <queue>


#include <polarcode/construction/bhattacharrya.h>
#include <polarcode/decoding/decoder.h>
#include <polarcode/encoding/encoder.h>
#include <polarcode/errordetection/errordetector.h>

#include <signalprocessing/modulation/bpsk.h>
#include <signalprocessing/random.h>

#include <signalprocessing/transmission/awgn.h>
#include <signalprocessing/transmission/scale.h>

#include "setup.h"
#include "statistics.h"

namespace Simulation {

inline int errorDetectionStringToId(std::string str)
{
    if (str == "crc8") {
        return 8;
    } else if (str == "crc32") {
        return 32;
    } else if (str == "cmac8") {
        return 8;
    } else if (str == "cmac16") {
        return 16;
    } else if (str == "cmac32") {
        return 32;
    } else if (str == "cmac64") {
        return 64;
    } else if (str == "cmac128") {
        return 128;
    } else {
        // implicit "none"
        return 0;
    }
}


inline std::string errorDetectionStringToType(std::string errDetStr)
{
    std::string prefixCRC = "crc";
    std::string prefixCMAC = "cmac";
    if (errDetStr.substr(0, prefixCRC.size()) == prefixCRC) {
        return prefixCRC;
    } else if (errDetStr.substr(0, prefixCMAC.size()) == prefixCMAC) {
        return prefixCMAC;
    } else { // implicit none!
        return std::string("none");
    }
}

/*!
 * \brief A collection of simulation input/output, called 'job'.
 *
 * This structure holds the parameters for a simulation job, including coding
 * parameters and statistical outputs after finishing the job.
 */
struct DataPoint {
    std::string name;

    // Codec-Parameters
    float designSNR;    ///< Design-SNR for code construction
    int N;              ///< Blocklength
    int K;              ///< Information length
    int L;              ///< List length for list-decoding
    int errorDetection; ///< 0=none,8/32=crc (effectively the number of check bits)
    std::string errorDetectionType;
    bool systematic; ///< True, if systematic coding will be used
    PolarCode::Decoding::DecoderType decoderType;
    int codingScheme; ///< -1 for flexible decoder, 0 or higher for fixed decoder
                      ///< according to _codeRegistry_
    std::vector<unsigned>
        scfNodeRanking; ///< SCFlip: Ranking of the worst info bit channels

    // Simulation-Parameters
    float EbN0;            ///< Bit-energy to noise-energy ratio for AWGN-channel
    long BlocksToSimulate; ///< Determines the BLER-precision
    int precision;         ///< Quantization bits per symbol (32-bit float or 8-bit int)
    float amplification;   ///< Amplification factor to optimize 8-bit quantization
    int bitsPerSymbol;

    // Statistics
    long runs;                  ///< Actual number of blocks simulated
    long bits;                  ///< Number of payload bits sent out
    long errors;                ///< ~ erroneous blocks
    long reportedErrors;        ///< ~ block errors reported by error detection
    long biterrors;             ///< ~ flipped payload bits
    float apparentlyBestMetric; ///< Lowest metric of output candidates (before error
                                ///< detection)
    float selectedPathMetric;   ///< Metric of the selected decoding path (after error
                                ///< detection)

    float BLER;          ///< Block Error Rate
    float BER;           ///< Bit Error Rate
    float RER;           ///< Reported Error Rate
    Statistics timeStat; ///< Decoding time in seconds
    StatisticsOutput time;
    float blps;          ///< Blocks per second
    float cbps;          ///< Channel bits per second
    float pbps;          ///< Payload bits per second (including errors)
    float effectiveRate; ///< Successfully transmitted payload bits per second
    float encTime;       ///< Encoding time in seconds
    float ebps;          ///< Encoder speed in bits per second
};

/*!
 * \brief The Simulator class
 */
class Simulator
{
    Setup::Configurator* mConfiguration;

    std::vector<DataPoint*> mJobList;
    std::atomic<unsigned> mNextJob;

    DataPoint* getDefaultDataPoint();
    void configureSingleRun();
    void configureCodeLengthSim();
    void configureDesignSnrSim();
    void configureListLengthSim();
    void configureRateSim();
    void configureAmplificationSim();
    void configureFixedSim();
    void configureDepthFirstSim();
    void configureScanSim(bool fastSimplified);
    void configureAskSim();
    void snrInflateJobList();
    void configureComparisonSim();
    void printCode();

    void saveResults();
    void saveComparisonResults();

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
class SimulationWorker
{
    Simulator* mSim;
    DataPoint* mJob;
    PolarCode::Construction::Bhattacharrya* mConstructor;
    PolarCode::Encoding::Encoder* mEncoder;
    PolarCode::Decoding::Decoder* mDecoder;
    PolarCode::ErrorDetection::Detector* mErrorDetector;

    SignalProcessing::Modulation::Modem *mModulator, *mDemodulator;
    SignalProcessing::Transmission::Awgn* mTransmitter;
    SignalProcessing::Transmission::Scale* mAmplifier;

    std::vector<unsigned> mFrozenBits;

    unsigned char* mInputData;
    PolarCode::PackedContainer* mEncodedData;
    std::vector<float>* mSignal;
    unsigned char* mDecodedData;

    std::chrono::high_resolution_clock::time_point mTimeStart, mTimeEnd;
    std::chrono::duration<float> mTimeUsed;

    int mWorkerId;
    bool warmup;

    void startTiming();
    void stopTiming();

    void selectFrozenBits();
    void setCoders();
    void setErrorDetector();
    void setChannel();
    void allocateMemory();

    void generateData();
    void encode();
    void modulate();
    void transmit();
    void demodulate();
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

} // namespace Simulation

#endif // PCSIM_SIMULATOR_H
