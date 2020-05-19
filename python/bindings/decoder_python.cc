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

#include <polarcode/decoding/decoder.h>
#include <polarcode/errordetection/errordetector.h>


namespace py = pybind11;


void bind_decoder(py::module& m)
{
    using namespace PolarCode::Decoding;
    using namespace PolarCode::ErrorDetection;
    py::class_<Decoder>(m, "PolarDecoder")
        .def(py::init(&PolarCode::Decoding::create))
        .def("blockLength", &Decoder::blockLength)
        .def("infoLength", &Decoder::infoLength)
        .def("setSystematic", &Decoder::setSystematic)
        .def("isSystematic", &Decoder::isSystematic)
        .def("frozenBits", &Decoder::frozenBits)
        .def("getErrorDetectionMode", &Decoder::getErrorDetectionMode)
        .def("setErrorDetection", [](Decoder& self, unsigned size, std::string type){
            self.setErrorDetection(PolarCode::ErrorDetection::create(size, type));
        },
        py::arg("size") = 0,
        py::arg("type") = "crc")
        .def("decode_vector",
             [](Decoder& self,
                const py::array_t<float, py::array::c_style |
                                         py::array::forcecast> array) {
            py::buffer_info inb = array.request();
            if(inb.ndim != 1){
                throw std::runtime_error("Only ONE-dimensional vectors allowed!");
            }
            if((size_t)inb.size != self.blockLength()){
                throw std::runtime_error("Input vector size != blockSize // 8!");
            }
            auto result =py::array_t<uint8_t>(self.infoLength() / 8);
            py::buffer_info resb = result.request();

            self.decode_vector((float*)inb.ptr, (void*)resb.ptr);
            return result;
        })
        .def("decode_vector",
             [](Decoder& self,
                const py::array_t<int8_t, py::array::c_style |
                                         py::array::forcecast> array) {
            py::buffer_info inb = array.request();
            if(inb.ndim != 1){
                throw std::runtime_error("Only ONE-dimensional vectors allowed!");
            }
            if((size_t)inb.size != self.blockLength()){
                throw std::runtime_error("Input vector size != blockSize // 8!");
            }
            auto result =py::array_t<uint8_t>(self.infoLength() / 8);
            py::buffer_info resb = result.request();

            self.decode_vector((char*)inb.ptr, (void*)resb.ptr);
            return result;
        })
        ;
}
