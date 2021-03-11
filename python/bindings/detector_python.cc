/*
 * Copyright 2020, 2021 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/complex.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdint>
#include <cstring>

#include <polarcode/errordetection/errordetector.h>

namespace py = pybind11;

void bind_detector(py::module& m)
{
    using namespace PolarCode::ErrorDetection;
    py::class_<Detector>(m, "Detector")
        .def(py::init(&create), py::arg("size"), py::arg("type"))
        .def("getCheckBitCount", &Detector::getCheckBitCount)
        .def("getType", &Detector::getType)
        .def(
            "calculate",
            [](Detector& self,
               const py::array_t<uint8_t, py::array::c_style | py::array::forcecast>&
                   array,
               const size_t bitsize) {
                py::buffer_info inb = array.request();
                if (inb.ndim != 1) {
                    throw std::runtime_error("Only ONE-dimensional vectors allowed!");
                }
                if (size_t(inb.size * inb.itemsize * 8) < bitsize) {
                    throw std::runtime_error("Provided number of bits is smaller than "
                                             "stated significant bits!");
                }

                return self.calculate((void*)inb.ptr, bitsize);
            },
            py::arg("data"),
            py::arg("bitsize"))
        .def("generate",
             [](Detector& self,
                const py::array_t<uint8_t, py::array::c_style | py::array::forcecast>&
                    array) {
                 py::buffer_info inb = array.request();
                 if (inb.ndim != 1) {
                     throw std::runtime_error("Only ONE-dimensional vectors allowed!");
                 }

                 unsigned crc_bytes = (self.getCheckBitCount() % 8)
                                          ? ((self.getCheckBitCount() + 8) / 8)
                                          : (self.getCheckBitCount() / 8);

                 auto result = py::array_t<uint8_t>(inb.size + crc_bytes);
                 py::buffer_info resb = result.request();
                 std::memcpy(resb.ptr, inb.ptr, inb.size * inb.itemsize);

                 self.generate((void*)resb.ptr, resb.size);
                 return result;
             })
        .def("check",
             [](Detector& self,
                const py::array_t<uint8_t, py::array::c_style | py::array::forcecast>&
                    array) {
                 py::buffer_info inb = array.request();
                 if (inb.ndim != 1) {
                     throw std::runtime_error("Only ONE-dimensional vectors allowed!");
                 }

                 return self.check((void*)inb.ptr, inb.size * inb.itemsize);
             });
}
