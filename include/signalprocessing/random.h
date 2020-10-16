/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PCDSP_RANDOM_H
#define PCDSP_RANDOM_H
//#undef __RDRND__

#include "lcg.h"
#include <alignednew.h>
#include <mutex>


namespace SignalProcessing {
namespace Random {

/*!
 * \brief Pseudo-random number generator
 */
class Generator : public AlignedNew<32>
{
#ifndef __RDRND__
#warning RDRND not used
    struct {
        LCG<uint64_t> Generator; ///< Linear congruential generator
        std::mutex Mutex;        ///< Mutex for multi-threading compatability
    } Uniform;
#endif

    struct {
        LCG<__m256> Generator; ///< Linear congruential generator
        std::mutex Mutex;      ///< Mutex for multi-threading compatability
    } Normal;

public:
    Generator();
    ~Generator();

    void get(uint32_t* ptr);     ///< Get an unsigned 32-bit integer
    void get64(uint64_t* ptr);   ///< Get an unsigned 64-bit integer
    void get64x4(uint64_t* ptr); ///< Get four unsigned 64-bit integers

    /*!
     * \brief Get two AVX-vectors á 8 normal distributed random numbers.
     * \param a Pointer to first destination.
     * \param b Pointer to second destination.
     */
    void getNormDist(__m256* a, __m256* b);

    /*!
     * \brief Get two vectors of Rayleigh distributed values.
     * \param a Pointer to first destination.
     * \param b Pointer to second destination.
     */
    void getRayleighDist(__m256* a, __m256* b);
};


} // namespace Random
} // namespace SignalProcessing

#endif
