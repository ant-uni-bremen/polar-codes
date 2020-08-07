# Copyright 2018 Florian Lotze
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

add_executable (errorlocator main setup simulator statistics)
target_link_libraries(errorlocator pthread PolarCode SignalProcessing)
