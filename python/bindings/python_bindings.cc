
/*
 * Copyright 2020 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/pybind11.h>
#include <pybind11/complex.h>
#include <pybind11/stl.h>


namespace py = pybind11;


void bind_puncturer(py::module& m);
void bind_encoder(py::module& m);


PYBIND11_MODULE(polarcode_python, m) {
    bind_puncturer(m);
    bind_encoder(m);


#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif

}

