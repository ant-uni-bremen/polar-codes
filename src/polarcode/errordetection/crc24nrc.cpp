/* -*- c++ -*- */
/*
 * Copyright 2021 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/errordetection/crc24nrc.h>
#define CRCPP_USE_CPP11
#define CRCPP_INCLUDE_ESOTERIC_CRC_DEFINITIONS
#include "CRC.h"


namespace PolarCode {
namespace ErrorDetection {


CRC24NRC::CRC24NRC() {}

CRC24NRC::~CRC24NRC() {}

uint32_t CRC24NRC::gen(uint32_t* data, int byteSize)
{
    return CRC::Calculate(data, byteSize, CRC::CRC_24_NRC());
}

bool CRC24NRC::check(void* pData, int bytes)
{
    uint32_t* data = reinterpret_cast<uint32_t*>(pData);
    uint8_t* p = reinterpret_cast<uint8_t*>(pData);
    const uint32_t rx_checksum = (uint32_t(p[bytes - 3] << 16) |
                                  uint32_t(p[bytes - 2] << 8) | uint32_t(p[bytes - 1]));
    const auto checksum = gen(data, bytes - 3);
    return checksum == rx_checksum;
}

void CRC24NRC::generate(void* pData, int bytes)
{
    uint32_t* data = reinterpret_cast<uint32_t*>(pData);
    auto checksum = gen(data, bytes - 3);

    uint8_t* p = reinterpret_cast<uint8_t*>(pData);
    p[bytes - 3] = (checksum >> 16) & 0xFF;
    p[bytes - 2] = (checksum >> 8) & 0xFF;
    p[bytes - 1] = checksum & 0xFF;
}


int CRC24NRC::multiCheck(void** pData, int nArrays, int nBytes)
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
