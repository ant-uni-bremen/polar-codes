
/*
 * Copyright 2020 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/pybind11.h>
#include <pybind11/complex.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

// #define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
// #include <numpy/arrayobject.h>

#include <cstdint>

#include <polarcode/encoding/encoder.h>
#include <polarcode/encoding/butterfly_fip_packed.h>
#include <polarcode/errordetection/errordetector.h>
#include <polarcode/errordetection/dummy.h>
#include <polarcode/errordetection/crc8.h>
#include <polarcode/errordetection/crc32.h>

namespace py = pybind11;


void bind_encoder(py::module& m)
{
    using namespace PolarCode::Encoding;
    using namespace PolarCode::ErrorDetection;
    py::class_<ButterflyFipPacked>(m, "PolarEncoder")
        .def(py::init<size_t, std::vector<unsigned> >())
        .def("blockLength", &Encoder::blockLength)
        .def("infoLength", &Encoder::infoLength)
        .def("setSystematic", &Encoder::setSystematic)
        .def("isSystematic", &Encoder::isSystematic)
        .def("frozenBits", &Encoder::frozenBits)
        .def("setErrorDetection", [](ButterflyFipPacked& self, unsigned size, std::string type){
            std::transform(type.begin(), type.end(), type.begin(),
                           [](unsigned char c){return std::tolower(c); } );
            if(type.find("crc") != std::string::npos){
                switch (size) {
                case 0: self.setErrorDetection(new Dummy()); break;
                case 8: self.setErrorDetection(new CRC8()); break;
                case 32: self.setErrorDetection(new CRC32()); break;
                default: self.setErrorDetection(new Dummy());
                }
            }
            else if(type.find("cmac") != std::string::npos){
                throw std::logic_error("CMAC INTERFACE NOT IMPLEMENTED");
            }
            else{
                throw std::runtime_error("Unknown Error detector requested!");
            }
        },
        py::arg("size") = 0,
        py::arg("type") = "crc")
        .def("encode_vector",
             [](ButterflyFipPacked& self,
                const py::array_t<uint8_t, py::array::c_style |
                                           py::array::forcecast> array) {
            py::buffer_info inb = array.request();
            if(inb.ndim != 1){
                throw std::runtime_error("Only ONE-dimensional vectors allowed!");
            }
            if((size_t)inb.size != self.infoLength() / 8){
                throw std::runtime_error("Input vector size != blockSize // 8!");
            }
            auto result =py::array_t<uint8_t>(self.blockLength() / 8);
            py::buffer_info resb = result.request();

            self.encode_vector((void*) inb.ptr, (void*) resb.ptr);
            return result;
        })
        ;
}


// cdef extern from "polarcode/encoding/encoder.h"
// namespace "PolarCode::Encoding":
//     cdef cppclass Encoder:
//         Encoder() except +
//         void encode()
//         void encode_vector(void*, void*)
//         void setInformation(void*)
//         void getEncodedData(void*)
//         size_t blockLength()
//         void setErrorDetection(Detector*)
//         void setSystematic(bool)
//         bool isSystematic()
//         vector[unsigned int] frozenBits()
//         size_t duration_ns()