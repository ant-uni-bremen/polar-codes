/* -*- c++ -*- */
/*
 * Copyright 2018, 2020 Florian Lotze, Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/errordetection/cmac.h>
#include <polarcode/errordetection/crc11nr.h>
#include <polarcode/errordetection/crc16.h>
#include <polarcode/errordetection/crc16nr.h>
#include <polarcode/errordetection/crc24nrc.h>
#include <polarcode/errordetection/crc32.h>
#include <polarcode/errordetection/crc6nr.h>
#include <polarcode/errordetection/crc8.h>
#include <polarcode/errordetection/dummy.h>
#include <polarcode/errordetection/errordetector.h>

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace PolarCode {
namespace ErrorDetection {

Detector* create(unsigned size, std::string type)
{
    std::transform(type.begin(), type.end(), type.begin(), [](unsigned char c) {
        return std::tolower(c);
    });

    Detector* detector;
    if (type.find("crc") != std::string::npos) {
        const bool use_5gnr_type = type.find("nr") != std::string::npos;

        std::vector<unsigned> crc_sizes({ 0, 6, 8, 11, 16, 24, 32 });
        bool found =
            (std::find(crc_sizes.begin(), crc_sizes.end(), size) != crc_sizes.end());
        if (not found) {
            throw std::logic_error("CRC INVALID SIZE!");
        }

        switch (size) {
        case 0:
            detector = new Dummy();
            break;
        case 6:
            detector = new CRC6NR();
            break;
        case 8:
            detector = new CRC8();
            break;
        case 11:
            detector = new CRC11NR();
            break;
        case 16:
            if (use_5gnr_type) {
                detector = new CRC16NR();
            } else {
                detector = new CRC16();
            }
            break;
        case 24:
            detector = new CRC24NRC();
            break;
        case 32:
            detector = new CRC32();
            break;
        default:
            detector = new Dummy();
        }
    } else if (type.find("cmac") != std::string::npos) {
        const unsigned char e_key[] = { 0x8e, 0x73, 0xb0, 0xf7, 0xda, 0x0e, 0x64, 0x52,
                                        0xc8, 0x10, 0xf3, 0x2b, 0x80, 0x90, 0x79, 0xe5 };
        std::vector<unsigned char> my_key(e_key, e_key + 16);
        std::vector<unsigned> cmac_sizes({ 8, 16, 32, 64, 128 });
        bool found =
            (std::find(cmac_sizes.begin(), cmac_sizes.end(), size) != cmac_sizes.end());
        if (not found) {
            throw std::logic_error("CMAC INVALID SIZE!");
        }
        detector = new PolarCode::ErrorDetection::cmac(my_key, size);
    } else {
        throw std::runtime_error("Unknown Error detector requested!");
    }
    return detector;
}

// The Detector-class is purely virtual.

} // namespace ErrorDetection
} // namespace PolarCode
