/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_ERR_CRC32_H
#define PC_ERR_CRC32_H

#include <polarcode/errordetection/errordetector.h>

namespace PolarCode {
namespace ErrorDetection {

/**
 * \brief The CRC32 class, using SSE4.2 instructions
 *
 * This class implements a 32-bit Cyclic Redundancy Check.
 * Calculations are done by utilizing SSE4.2 instruction _mm_crc32_u32().
 * See [Intel Intrinsics
 * Guide](https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=crc&expand=1227)
 */
class CRC32 : public Detector
{
    void checkBlockSizeRestriction(int blockCount, int byteCount);
    unsigned int gen(unsigned int* data, int blockCount);

public:
    CRC32();
    ~CRC32();

    std::string getType() { return std::string("CRC"); }
    unsigned getCheckBitCount() { return 32; }
    uint64_t calculate(void* data, size_t bits);
    void generate(void* pData, int bytes);
    bool check(void* pData, int bytes);
    int multiCheck(void** dataPtr, int nArrays, int nBytes);
};

} // namespace ErrorDetection
} // namespace PolarCode

#endif // PC_ERR_CRC32_H
