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
    crc6nr.h
    crc8.h
    crc11nr.h
    crc16.h
    crc16nr.h
    crc24nrc.h
    crc32.h
    dummy.h DESTINATION include/polarcode/errordetection
)
