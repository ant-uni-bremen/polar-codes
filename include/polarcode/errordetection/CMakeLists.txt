# Copyright 2018 Florian Lotze
#
# This file is part of PolarCodes
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Install public header files
########################################################################
install(FILES
    errordetector.h
    crc8.h
    crc16.h
    crc32.h
    dummy.h DESTINATION include/polarcode/errordetection
)
