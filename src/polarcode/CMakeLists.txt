# Copyright 2018 Florian Lotze
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

add_library(ErrorDetector OBJECT
        errordetection/errordetector
        errordetection/dummy
        errordetection/crc8
        errordetection/crc16
        errordetection/crc32
        errordetection/cmac
        ${CMAKE_SOURCE_DIR}/include/polarcode/errordetection/errordetector.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/errordetection/dummy.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/errordetection/crc8.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/errordetection/crc16.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/errordetection/crc32.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/errordetection/cmac.h)

add_library(PolarEncoder OBJECT
        encoding/encoder
        encoding/butterfly_fip
        encoding/butterfly_fip_packed
        encoding/recursive_fip_packed
        ${CMAKE_SOURCE_DIR}/include/polarcode/encoding/encoder.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/encoding/butterfly_fip.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/encoding/butterfly_fip_packed.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/encoding/recursive_fip_packed.h)

add_library(PolarConstructor OBJECT
            construction/constructor
            construction/bhattacharrya
            ${CMAKE_SOURCE_DIR}/include/polarcode/construction/constructor.h
            ${CMAKE_SOURCE_DIR}/include/polarcode/construction/bhattacharrya.h)


#add_executable(pcfactory
#    $<TARGET_OBJECTS:PolarConstructor>
#    arrayfuncs.cpp
#    decoding/decoderfactory/main)

##First compilation: Declare, that this command creates the missing file fixeddecoders.cpp
#ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_SOURCE_DIR}/src/polarcode/decoding/decoderfactory/fixeddecoders.cpp
#    DEPENDS PolarConstructor
#    COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pcfactory" "${CMAKE_SOURCE_DIR}/src/polarcode/decoding/decoderfactory/fixeddecoders.cpp")

##Following compilations: Declare, that this command should be run, if the factory has been rebuilt, e.g. after changing MCSs.
#ADD_CUSTOM_COMMAND(TARGET pcfactory POST_BUILD
#    DEPENDS PolarConstructor
#    COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pcfactory" "${CMAKE_SOURCE_DIR}/src/polarcode/decoding/decoderfactory/fixeddecoders.cpp")


add_library(PolarDecoder OBJECT
        decoding/decoder
        decoding/errorlocator
        decoding/fastssc_fip_char
        decoding/scl_fip_char
        decoding/fastssc_avx_float
        decoding/scl_avx_float
#        decoding/fixed_fip_char
        decoding/adaptive_float
        decoding/adaptive_char
        decoding/adaptive_mixed
        decoding/depth_first
        decoding/scan
        decoding/fastsscan_float
#        ${CMAKE_SOURCE_DIR}/src/polarcode/decoding/decoderfactory/fixeddecoders
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/decoder.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/errorlocator.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/fip_char.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/fip_templates.txx
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/fastssc_fip_char.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/scl_fip_char.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/avx_float.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/fastssc_avx_float.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/scl_avx_float.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/adaptive_float.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/adaptive_char.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/adaptive_mixed.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/depth_first.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/templatized_float.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/scan.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/fastsscan_float.h)

add_library(PolarCode
        $<TARGET_OBJECTS:PolarConstructor>
        $<TARGET_OBJECTS:PolarEncoder>
        $<TARGET_OBJECTS:PolarDecoder>
        $<TARGET_OBJECTS:ErrorDetector>
        avxconvenience
        arrayfuncs
        bitcontainer
        polarcode
        puncturer
        ${CMAKE_SOURCE_DIR}/include/polarcode/avxconvenience.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/arrayfuncs.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/bitcontainer.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/datapool.txx
        ${CMAKE_SOURCE_DIR}/include/polarcode/polarcode.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/puncturer.h)

target_link_libraries(PolarCode ssl crypto)

target_compile_features(PolarCode PRIVATE cxx_std_14)

include(GNUInstallDirs)
install(TARGETS PolarCode
        EXPORT polarcode-targets
#        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
#        ARCHIVE DESTINATION lib/static)
)

install(EXPORT polarcode-targets
    FILE
        PolarCodeTargets.cmake
    DESTINATION
        lib/cmake/PolarCode
)
# ${CMAKE_MODULES_DIR}/cmake/PolarCode
