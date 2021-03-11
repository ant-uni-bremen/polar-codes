/* -*- c++ -*- */
/*
 * Copyright 2021 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_ERR_CRC24NRC_H
#define PC_ERR_CRC24NRC_H

#include <polarcode/errordetection/errordetector.h>

namespace PolarCode {
namespace ErrorDetection {

/**
 * \brief The CRC24C NR class
 *
 * Polynomial: 0xb2b117
 * Input reflected: false
 * Output reflected: false
 * Initial value: 0x0000
 * Final XOR value: 0x0000
 *
 * CRC24 NR C
 * cf. 3GPP TS 38.212 Sec. 5.1
 */
class CRC24NRC : public Detector
{
    uint32_t gen(uint32_t* data, size_t bitSize);

public:
    CRC24NRC();
    ~CRC24NRC();

    std::string getType() { return std::string("CRCNR"); }
    unsigned getCheckBitCount() { return 24; }
    uint64_t calculate(void* data, size_t bits);
    void generate(void* pData, int bytes);
    bool check(void* pData, int bytes);
    int multiCheck(void** dataPtr, int nArrays, int nBytes);
};

} // namespace ErrorDetection
} // namespace PolarCode

#endif // PC_ERR_CRC24NRC_H
