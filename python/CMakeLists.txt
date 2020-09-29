# Copyright 2020 Johannes Demel
#
# SPDX-License-Identifier: GPL-3.0-or-later
#


########################################################################
# Install python sources
########################################################################
install(
    FILES
    __init__.py
    frozen_bit_positions.py
    channel_construction.py
    5g_polar_code_reliability_table.csv
    DESTINATION ${PYTHON_DIR}/pypolar
)

add_subdirectory(bindings)
