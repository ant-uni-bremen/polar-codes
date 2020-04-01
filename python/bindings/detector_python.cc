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
#include <cstring>

#include <polarcode/errordetection/errordetector.h>


namespace py = pybind11;


void bind_detector(py::module& m)
{
    using namespace PolarCode::ErrorDetection;
    py::class_<Detector>(m, "Detector")
        .def(py::init(&create))
        .def("getCheckBitCount", &Detector::getCheckBitCount)
        .def("generate",
             [](Detector& self,
                const py::array_t<uint8_t, py::array::c_style |
                                         py::array::forcecast> &array) {
            py::buffer_info inb = array.request();
            if(inb.ndim != 1){
                throw std::runtime_error("Only ONE-dimensional vectors allowed!");
            }

            auto result =py::array_t<uint8_t>(inb.size + self.getCheckBitCount() / 8);
            py::buffer_info resb = result.request();
            std::memcpy(resb.ptr, inb.ptr, inb.size * inb.itemsize);

            self.generate((void*)resb.ptr, resb.size);
            return result;
        })
        .def("check",
             [](Detector& self,
                const py::array_t<uint8_t, py::array::c_style |
                                         py::array::forcecast> &array) {
            py::buffer_info inb = array.request();
            if(inb.ndim != 1){
                throw std::runtime_error("Only ONE-dimensional vectors allowed!");
            }

            return self.check((void*)inb.ptr, inb.size * inb.itemsize);
        })
        ;
}

