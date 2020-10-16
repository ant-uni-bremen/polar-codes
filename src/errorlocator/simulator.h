/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PCERL_SIMULATOR_H
#define PCERL_SIMULATOR_H

#include <atomic>
#include <chrono>
#include <mutex>
#include <queue>
#include <vector>


#include <polarcode/construction/bhattacharrya.h>
#include <polarcode/decoding/errorlocator.h>
#include <polarcode/encoding/encoder.h>

#include <signalprocessing/modulation/bpsk.h>
#include <signalprocessing/random.h>

#include <signalprocessing/transmission/awgn.h>
#include <signalprocessing/transmission/scale.h>

#include "setup.h"
#include "statistics.h"

namespace SimulationErrorLocator {

/*!
 * \brief A collection of simulation input/output, called 'job'.
 *
 * This structure holds the parameters for a simulation job, including coding
 * parameters and statistical outputs after finishing the job.
 */
struct DataPoint {
    std::mutex mutex;

    // Codec-Parameters
    float designSNR; ///< Design-SNR for code construction
    int N;           ///< Blocklength
    int K;           ///< Information length
    std::vector<bool> frozenSet;

    // Simulation-Parameters
    float EbN0;            ///< Bit-energy to noise-energy ratio for AWGN-channel
    long BlocksToSimulate; ///< Determines the BLER-precision

    // Statistics
    int runs;
    std::vector<int> firstErrorPosition;
    std::vector<Statistics> additionalErrors;
};

/*!
 * \brief The Simulator class
 */
class Simulator
{
    Setup::Configurator* mConfiguration;
    DataPoint* mJob;
    std::map<int, std::map<int, int>> mErrorCounter;

    void configure();
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

    void incrementErrorCounter(int numErrors, int firstErrorPosition);
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
    PolarCode::Decoding::ErrorLocator *mDecoder, *mReferenceDecoder;

    SignalProcessing::Modulation::Modem *mModulator, *mDemodulator;
    SignalProcessing::Transmission::Awgn* mTransmitter;
    SignalProcessing::Transmission::Scale* mAmplifier;

    std::vector<unsigned> mFrozenBits;

    unsigned char* mInputData;
    PolarCode::PackedContainer* mEncodedData;
    std::vector<float>* mSignal;
    std::vector<float> mReferenceSignal;
    unsigned char* mDecodedData;

    int mWorkerId;

    void selectFrozenBits();
    void setCoders();
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

} // namespace SimulationErrorLocator

#endif // PCERL_SIMULATOR_H
