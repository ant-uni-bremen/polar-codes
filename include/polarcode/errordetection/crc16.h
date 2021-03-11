/* -*- c++ -*- */
/*
 * Copyright 2020 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_ERR_CRC16_H
#define PC_ERR_CRC16_H

#include <polarcode/errordetection/errordetector.h>

namespace PolarCode {
namespace ErrorDetection {

/**
 * \brief The CRC16 CCITTFALSE class
 *
 * Polynomial: 0x1021
 * Input reflected: false
 * Output reflected: false
 * Initial value: 0xFFFF
 * Final XOR value: 0x0000
 *
 * The initial 0xFFFF helps to avoid misdetection for all zero packets.
 */
class CRC16 : public Detector
{
    uint16_t gen(uint16_t* data, int byteSize);

public:
    CRC16();
    ~CRC16();

    std::string getType() { return std::string("CRC"); }
    unsigned getCheckBitCount() { return 16; }
    uint64_t calculate(void* data, size_t bits);
    void generate(void* pData, int bytes);
    bool check(void* pData, int bytes);
    int multiCheck(void** dataPtr, int nArrays, int nBytes);
};

} // namespace ErrorDetection
} // namespace PolarCode

#endif // PC_ERR_CRC16_H
