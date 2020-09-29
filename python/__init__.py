#
# Copyright 2020 Johannes Demel
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

# The presence of this file turns this directory into a Python package

'''
This is the PolarCode module.

It is a wrapper for C++ PolarCode stitched together with PyBIND11
'''

# import pybind11 module.
from .polarcode_python import *



# import any pure python here
from .frozen_bit_positions import get_frozen_bit_generator
