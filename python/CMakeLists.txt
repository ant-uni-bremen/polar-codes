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

add_test(NAME "pypolar_encoder" COMMAND python3 qa_pypolar_encoder.py WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/python)
add_test(NAME "pypolar_decoder" COMMAND python3 qa_pypolar_decoder.py WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/python)
add_test(NAME "pypolar_detector" COMMAND python3 qa_pypolar_detector.py WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/python)
add_test(NAME "pypolar_puncturer" COMMAND python3 qa_pypolar_puncturer.py WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/python)
