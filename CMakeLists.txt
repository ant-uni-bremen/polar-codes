project ("Polar Coding Simulation")
cmake_minimum_required(VERSION 2.8)
find_package(Doxygen)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")
find_package(CppUnit)
#find_package(Threads)

SET(BUILD_SHARED_LIBS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_definitions(-std=c++11 -Wall -Wno-ignored-attributes)
add_definitions(-march=native -fPIC)
add_definitions(-O2 -funroll-loops)
#add_definitions(-Wl,--no-as-needed)

add_subdirectory(src)
add_subdirectory(test)
#add_subdirectory(doc)
add_subdirectory(include/polarcode)


########################################################################
# Setup the package config file
########################################################################
#set variables found in the pc.in file
set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/lib${LIB_SUFFIX}")
set(includedir "\${prefix}/include")
set(grcdir "\${prefix}/share/gnuradio/grc/blocks/")
set(GR_PKG_DATA_DIR "\${prefix}/share/gnuradio/")
set(GRC_BLOCKS_DIR      ${GR_PKG_DATA_DIR}/grc/blocks)
set(GR_LIBRARY_DIR lib${LIB_SUFFIX})

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/PolarCode.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/PolarCode.pc
@ONLY)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/PolarCode.pc
    DESTINATION lib${LIB_SUFFIX}/pkgconfig
    COMPONENT "PolarCode_devel"
)
