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
        encoding/butterfly_avx2
        encoding/butterfly_avx2_packed
        encoding/recursive_avx2_packed
        ${CMAKE_SOURCE_DIR}/include/polarcode/encoding/encoder.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/encoding/butterfly_avx2.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/encoding/butterfly_avx2_packed.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/encoding/recursive_avx2_packed.h)

add_library(PolarConstructor OBJECT
            construction/constructor
            construction/bhattacharrya
            ${CMAKE_SOURCE_DIR}/include/polarcode/construction/constructor.h
            ${CMAKE_SOURCE_DIR}/include/polarcode/construction/bhattacharrya.h)


add_executable(pcfactory
    decoding/decoderfactory/main)

#First compilation: Declare, that this command creates the missing file fixeddecoders.cpp
ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_SOURCE_DIR}/src/polarcode/decoding/decoderfactory/fixeddecoders.cpp
    DEPENDS PolarConstructor
    COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pcfactory" "${CMAKE_SOURCE_DIR}/src/polarcode/decoding/decoderfactory/fixeddecoders.cpp")

#Following compilations: Declare, that this command should be run, if the factory has been rebuilt, e.g. after changing MCSs.
ADD_CUSTOM_COMMAND(TARGET pcfactory POST_BUILD
    DEPENDS PolarConstructor
    COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pcfactory" "${CMAKE_SOURCE_DIR}/src/polarcode/decoding/decoderfactory/fixeddecoders.cpp")


add_library(PolarDecoder OBJECT
        decoding/decoder
        decoding/fastssc_avx2_char
        decoding/scl_avx2_char
        decoding/fastssc_avx_float
        decoding/scl_avx_float
        decoding/adaptive_float
        decoding/adaptive_char
        decoding/adaptive_mixed
        ${CMAKE_SOURCE_DIR}/src/polarcode/decoding/decoderfactory/fixeddecoders
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/decoder.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/avx2_char.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/fastssc_avx2_char.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/scl_avx2_char.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/avx_float.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/fastssc_avx_float.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/scl_avx_float.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/adaptive_float.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/adaptive_char.h
        ${CMAKE_SOURCE_DIR}/include/polarcode/decoding/adaptive_mixed.h)

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
        ${CMAKE_SOURCE_DIR}/include/polarcode/datapool.txx
        ${CMAKE_SOURCE_DIR}/include/polarcode/polarcode.h)

install(TARGETS PolarCode
#        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
#        ARCHIVE DESTINATION lib/static)
)
