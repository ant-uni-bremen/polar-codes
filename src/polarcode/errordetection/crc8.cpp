/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/errordetection/crc8.h>

//#define GP  0x107   /* x^8 + x^2 + x + 1 */
//#define DI  0x07

namespace PolarCode {
namespace ErrorDetection {


CRC8::CRC8()
{
    int i, j;
    unsigned char crc;

    for (i = 0; i < 256; i++) {
        crc = i;
        for (j = 0; j < 8; j++) {
            crc = (crc << 1) ^ ((crc & 0x80) ? /*DI*/ 0x07 : 0);
        }
        table[i] = crc & 0xFF;
    }
}

CRC8::~CRC8() {}

uint64_t CRC8::calculate(void* data, size_t bits)
{
    return gen(reinterpret_cast<unsigned char*>(data), bits / 8);
}

unsigned char CRC8::gen(unsigned char* data, int bytes)
{
    unsigned char chkSum = 0;
    for (int i = 0; i < bytes; ++i) {
        // gen(&ret, data[i]);
        chkSum = table[chkSum ^ data[i]];
    }
    return chkSum;
}

bool CRC8::check(void* pData, int bytes)
{
    unsigned char* data = reinterpret_cast<unsigned char*>(pData);
    unsigned char toCheck = gen(data, bytes - 1);
    return toCheck == data[bytes - 1];
}

void CRC8::generate(void* pData, int bytes)
{
    unsigned char* data = reinterpret_cast<unsigned char*>(pData);
    unsigned char chkSum = gen(data, bytes - 1);
    data[bytes - 1] = chkSum;
}


int CRC8::multiCheck(void** pData, int nArrays, int nBytes)
{
    unsigned char** data = reinterpret_cast<unsigned char**>(pData);
    unsigned char* checksums = new unsigned char[nArrays]();
    int nCheckBytes = nBytes - 1;

    for (int byte = 0; byte < nCheckBytes; ++byte) {
        for (int array = 0; array < nArrays; ++array) {
            checksums[array] = table[checksums[array] ^ data[array][byte]];
        }
    }

    int firstMatch = -1;
    for (int array = 0; array < nArrays; ++array) {
        if (checksums[array] == data[array][nBytes - 1]) {
            firstMatch = array;
            break;
        }
    }


    delete[] checksums;
    return firstMatch;
}


} // namespace ErrorDetection
} // namespace PolarCode
