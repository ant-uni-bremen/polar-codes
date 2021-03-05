/* -*- c++ -*- */
/*
 * Copyright 2021 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_ERR_CRC11NR_H
#define PC_ERR_CRC11NR_H

#include <polarcode/errordetection/errordetector.h>

namespace PolarCode {
namespace ErrorDetection {

/**
 * \brief The CRC11 NR class
 *
 * Polynomial: 0x621
 * Input reflected: false
 * Output reflected: false
 * Initial value: 0x0000
 * Final XOR value: 0x0000
 *
 * cf. 3GPP TS 38.212 Sec. 5.1
 */
class CRC11NR : public Detector
{
    uint16_t gen(uint16_t* data, int byteSize);

public:
    CRC11NR();
    ~CRC11NR();

    std::string getType() { return std::string("CRCNR"); }
    unsigned getCheckBitCount() { return 11; }
    void generate(void* pData, int bytes);
    bool check(void* pData, int bytes);
    int multiCheck(void** dataPtr, int nArrays, int nBytes);
};

} // namespace ErrorDetection
} // namespace PolarCode

#endif // PC_ERR_CRC11NR_H
