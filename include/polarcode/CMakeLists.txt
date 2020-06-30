# Copyright 2019, 2020 Florian Lotze, Johannes Demel
#
# This file is part of PolarCodes
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Install public header files
########################################################################
add_subdirectory(encoding)
add_subdirectory(decoding)
add_subdirectory(errordetection)
add_subdirectory(construction)

install(FILES
    bitcontainer.h
    puncturer.h DESTINATION include/polarcode
)
