/* -*- c++ -*- */
/*
 * Copyright 2021 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_ERR_CRC16NR_H
#define PC_ERR_CRC16NR_H

#include <polarcode/errordetection/errordetector.h>

namespace PolarCode {
namespace ErrorDetection {

/**
 * \brief The CRC16 NR (CRC16 XMODEM) class
 *
 * Polynomial: 0x1021
 * Input reflected: false
 * Output reflected: false
 * Initial value: 0x0000
 * Final XOR value: 0x0000
 *
 * CRC16 NR and CRC16 XMODEM are the same.
 * cf. 3GPP TS 38.212 Sec. 5.1
 * cf. 3GPP TS 36.212 Sec. 5.1 for LTE version.
 */
class CRC16NR : public Detector
{
    uint16_t gen(uint16_t* data, int byteSize);

public:
    CRC16NR();
    ~CRC16NR();

    std::string getType() { return std::string("CRCNR"); }
    unsigned getCheckBitCount() { return 16; }
    void generate(void* pData, int bytes);
    bool check(void* pData, int bytes);
    int multiCheck(void** dataPtr, int nArrays, int nBytes);
};

} // namespace ErrorDetection
} // namespace PolarCode

#endif // PC_ERR_CRC16NR_H
