/* -*- c++ -*- */
/*
 * Copyright 2021 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/errordetection/crc6nr.h>

#define CRCPP_USE_CPP11
#define CRCPP_INCLUDE_ESOTERIC_CRC_DEFINITIONS
#include "CRC.h"

namespace PolarCode {
namespace ErrorDetection {


CRC6NR::CRC6NR() {}

CRC6NR::~CRC6NR() {}

uint64_t CRC6NR::calculate(void* data, size_t bits)
{
    return gen(reinterpret_cast<uint8_t*>(data), bits);
}

uint8_t CRC6NR::gen(uint8_t* data, int bitSize)
{
    return CRC::CalculateBits(data, bitSize, CRC::CRC_6_NR());
}

bool CRC6NR::check(void* pData, int bytes)
{
    uint8_t* data = reinterpret_cast<uint8_t*>(pData);
    uint8_t* p = reinterpret_cast<uint8_t*>(pData);
    const uint8_t rx_checksum = uint8_t(p[bytes - 1]);
    const auto checksum = gen(data, 8 * (bytes - 1));
    return checksum == rx_checksum;
}

void CRC6NR::generate(void* pData, int bytes)
{
    uint8_t* data = reinterpret_cast<uint8_t*>(pData);
    auto checksum = gen(data, 8 * (bytes - 1));

    uint8_t* p = reinterpret_cast<uint8_t*>(pData);
    p[bytes - 1] = checksum & 0xFF;
}


int CRC6NR::multiCheck(void** pData, int nArrays, int nBytes)
{
    uint16_t** data = reinterpret_cast<uint16_t**>(pData);

    int firstMatch = -1;
    for (int array = 0; array < nArrays; ++array) {
        if (check(data[array], nBytes)) {
            firstMatch = array;
            break;
        }
    }

    return firstMatch;
}

} // namespace ErrorDetection
} // namespace PolarCode
