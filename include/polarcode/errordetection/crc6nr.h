/* -*- c++ -*- */
/*
 * Copyright 2021 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_ERR_CRC6NR_H
#define PC_ERR_CRC6NR_H

#include <polarcode/errordetection/errordetector.h>

namespace PolarCode {
namespace ErrorDetection {

/*!
 * \brief Error detection via eight bit Cyclic Redundancy Check
 *
 * Polynom: 0x21
 * Init: 0x00
 * XorOut: 0x00
 * RefIn: false
 * RefOut: false
 */
class CRC6NR : public Detector
{
    uint8_t gen(uint8_t* data, int bytes);

public:
    CRC6NR();
    ~CRC6NR();

    std::string getType() { return std::string("CRCNR"); }
    unsigned getCheckBitCount() { return 6; }
    void generate(void* pData, int bytes);
    bool check(void* pData, int bytes);
    int multiCheck(void** pData, int nArrays, int nBytes);
};

} // namespace ErrorDetection
} // namespace PolarCode

#endif // PC_ERR_CRC6NR_H
