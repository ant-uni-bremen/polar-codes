# Copyright 2020 Johannes Demel
#
# This file is part of PolarCodes
#
# SPDX-License-Identifier: GPL-3.0-or-later
#


add_executable (pcbench main_benchmark)

target_link_libraries(pcbench benchmark::benchmark PolarCode)
