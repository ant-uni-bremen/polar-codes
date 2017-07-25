add_definitions(-mavx2 -mrdrnd -mbmi) 

add_library(ErrorDetector OBJECT
        errordetection/errordetector
        errordetection/dummy
        errordetection/crc8
        errordetection/crc32
        ${CMAKE_SOURCE_DIR}/include/polarcode/errordetection/errordetector.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/errordetection/dummy.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/errordetection/crc8.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/errordetection/crc32.h)

add_library(PolarEncoder OBJECT
        encoding/encoder
        encoding/butterfly_avx_float
        encoding/butterfly_avx2_char
        ${CMAKE_SOURCE_DIR}/include/polarcode/encoding/encoder.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/encoding/butterfly_avx_float.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/encoding/butterfly_avx2_char.h)

add_library(PolarDecoder OBJECT
        decoding/decoder
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/decoder.h)

add_library(PolarConstructor OBJECT
        construction/constructor
        construction/bhattacharrya
        ${CMAKE_SOURCE_DIR}/include/polarcode/construction/constructor.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/construction/bhattacharrya.h)

add_library(PolarCode
        $<TARGET_OBJECTS:PolarConstructor>
        $<TARGET_OBJECTS:PolarEncoder>
        $<TARGET_OBJECTS:PolarDecoder>
        $<TARGET_OBJECTS:ErrorDetector>
        avxconvenience
        arrayfuncs
        bitcontainer
        polarcode
        ${CMAKE_SOURCE_DIR}/include/polarcode/avxconvenience.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/arrayfuncs.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/bitcontainer.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/polarcode.h)