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
    DESTINATION ${PYTHON_DIR}/pypolar
)

add_subdirectory(bindings)