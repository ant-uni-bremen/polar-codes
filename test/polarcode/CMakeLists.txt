# Copyright 2018 Florian Lotze
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

add_library(PolarTest OBJECT
        siformat
        polarcodetest
        puncturertest
        bitcontainertest
        errordetectiontest
        encodingtest
        decodingtest
        constructiontest)
#install_targets(polarcode-test ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
