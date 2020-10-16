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

#include <cstdint>

#include <polarcode/puncturer.h>

namespace py = pybind11;

void bind_puncturer(py::module &m)
{
    py::class_<PolarCode::Puncturer>(m, "Puncturer")
        .def(py::init<size_t, std::vector<unsigned>>(),
             py::arg("blockLength"),
             py::arg("frozenBitPositions"))
        .def("blockLength", &PolarCode::Puncturer::blockLength)
        .def("parentBlockLength", &PolarCode::Puncturer::parentBlockLength)
        .def("blockOutputPositions", &PolarCode::Puncturer::blockOutputPositions)
        .def("puncturePacked", [](PolarCode::Puncturer &self, const py::array_t<uint8_t, py::array::c_style | py::array::forcecast> array) {
            py::buffer_info inb = array.request();
            if (inb.ndim != 1)
            {
                throw std::runtime_error("Only ONE-dimensional vectors allowed!");
            }
            if ((size_t)inb.size != self.parentBlockLength() / 8)
            {
                throw std::runtime_error("Input vector size != parentBlockSize!");
            }
            auto result = py::array_t<uint8_t>(self.blockLength() / 8);
            py::buffer_info resb = result.request();

            self.puncturePacked((uint8_t *)resb.ptr, (uint8_t *)inb.ptr);
            return result;
        })
        .def("puncture", [](PolarCode::Puncturer &self, const py::array_t<double, py::array::c_style | py::array::forcecast> array) {
            py::buffer_info inb = array.request();
            if (inb.ndim != 1)
            {
                throw std::runtime_error("Only ONE-dimensional vectors allowed!");
            }
            if ((size_t)inb.size != self.parentBlockLength())
            {
                throw std::runtime_error("Input vector size != parentBlockSize!");
            }
            auto result = py::array_t<double>(self.blockLength());
            py::buffer_info resb = result.request();

            self.puncture<double>((double *)resb.ptr, (double *)inb.ptr);
            return result;
        })
        .def("puncture", [](PolarCode::Puncturer &self, const py::array_t<float, py::array::c_style | py::array::forcecast> array) {
            py::buffer_info inb = array.request();
            if (inb.ndim != 1)
            {
                throw std::runtime_error("Only ONE-dimensional vectors allowed!");
            }
            if ((size_t)inb.size != self.parentBlockLength())
            {
                throw std::runtime_error("Input vector size != parentBlockSize!");
            }
            auto result = py::array_t<float>(self.blockLength());
            py::buffer_info resb = result.request();

            self.puncture<float>((float *)resb.ptr, (float *)inb.ptr);
            return result;
        })
        .def("puncture", [](PolarCode::Puncturer &self, const py::array_t<uint8_t, py::array::c_style | py::array::forcecast> array) {
            py::buffer_info inb = array.request();
            if (inb.ndim != 1)
            {
                throw std::runtime_error("Only ONE-dimensional vectors allowed!");
            }
            if ((size_t)inb.size != self.parentBlockLength())
            {
                throw std::runtime_error("Input vector size != parentBlockSize!");
            }
            auto result = py::array_t<uint8_t>(self.blockLength());
            py::buffer_info resb = result.request();

            self.puncture<uint8_t>((uint8_t *)resb.ptr, (uint8_t *)inb.ptr);
            return result;
        })

        .def("depuncture", [](PolarCode::Puncturer &self, const py::array_t<double, py::array::c_style | py::array::forcecast> array) {
            py::buffer_info inb = array.request();
            if (inb.ndim != 1)
            {
                throw std::runtime_error("Only ONE-dimensional vectors allowed!");
            }
            if ((size_t)inb.size != self.blockLength())
            {
                throw std::runtime_error("Input vector size != blockSize!");
            }
            auto result = py::array_t<double>(self.parentBlockLength());
            py::buffer_info resb = result.request();

            self.depuncture<double>((double *)resb.ptr, (double *)inb.ptr);
            return result;
        })
        .def("depuncture", [](PolarCode::Puncturer &self, const py::array_t<float, py::array::c_style | py::array::forcecast> array) {
            py::buffer_info inb = array.request();
            if (inb.ndim != 1)
            {
                throw std::runtime_error("Only ONE-dimensional vectors allowed!");
            }
            if ((size_t)inb.size != self.blockLength())
            {
                throw std::runtime_error("Input vector size != blockSize!");
            }
            auto result = py::array_t<float>(self.parentBlockLength());
            py::buffer_info resb = result.request();

            self.depuncture<float>((float *)resb.ptr, (float *)inb.ptr);
            return result;
        })
        .def("depuncture", [](PolarCode::Puncturer &self, const py::array_t<uint8_t, py::array::c_style | py::array::forcecast> array) {
            py::buffer_info inb = array.request();
            if (inb.ndim != 1)
            {
                throw std::runtime_error("Only ONE-dimensional vectors allowed!");
            }
            if ((size_t)inb.size != self.blockLength())
            {
                throw std::runtime_error("Input vector size != bBlockSize!");
            }
            auto result = py::array_t<uint8_t>(self.parentBlockLength());
            py::buffer_info resb = result.request();

            self.depuncture<uint8_t>((uint8_t *)resb.ptr, (uint8_t *)inb.ptr);
            return result;
        })

        ;
}
