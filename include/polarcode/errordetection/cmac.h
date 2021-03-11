/* -*- c++ -*- */
/*
 * Copyright 2020 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef POLAR_CODING_SIMULATION_CMAC_H
#define POLAR_CODING_SIMULATION_CMAC_H

#include <polarcode/errordetection/errordetector.h>
#include <cstddef>
#include <vector>

namespace PolarCode {
namespace ErrorDetection {

/*!
 * \brief Error detection via variable length CMAC
 */
class cmac : public Detector
{
    unsigned char gen(unsigned char* data, int bytes);
    unsigned int mBitCount;
    unsigned char* mMacKey;

    size_t calculate_cmac_len(unsigned char* cmac,
                              const unsigned int cmac_len,
                              const unsigned char* key,
                              const unsigned int key_len,
                              const unsigned char* message,
                              const unsigned int msg_len);
    size_t calculate_cmac(unsigned char* cmac,
                          const unsigned char* key,
                          const unsigned int key_len,
                          const unsigned char* message,
                          const unsigned int msg_len);

public:
    cmac(std::vector<unsigned char> initKey, unsigned int bitCount = 128);
    ~cmac();
    void setKey(std::vector<unsigned char> key);
    std::string getType() { return std::string("CMAC"); }
    unsigned getCheckBitCount() { return mBitCount; }
    uint64_t calculate(void* data, size_t bits);
    void generate(void* pData, int bytes);
    bool check(void* pData, int bytes);
    int multiCheck(void** pData, int nArrays, int nBytes);
};

} // namespace ErrorDetection
} // namespace PolarCode


#endif // POLAR_CODING_SIMULATION_CMAC_H
