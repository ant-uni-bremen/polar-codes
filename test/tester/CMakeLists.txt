# Copyright 2018 Florian Lotze
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

add_executable(pctest
    tester
    $<TARGET_OBJECTS:PolarTest>
    $<TARGET_OBJECTS:SigProcTest>)

#ADD_CUSTOM_COMMAND(TARGET pctest POST_BUILD
#  COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pctest")
ADD_CUSTOM_TARGET(tests "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pctest"
    DEPENDS pctest PolarCode SignalProcessing PolarTest
    COMMENT "Running CPPUNIT tests...")

target_link_libraries(pctest PolarCode SignalProcessing cppunit fmt::fmt)

add_test(NAME "TestPolarCodeCPP" COMMAND pctest)
