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

#include <polarcode/construction/constructor.h>

namespace py = pybind11;

void bind_constructor(py::module &m)
{
    m.def("frozen_bits", &PolarCode::Construction::frozen_bits,
          py::arg("blockLength"), py::arg("infoLength"), py::arg("designSNR"));
}
