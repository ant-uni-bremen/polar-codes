########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 3.8)
project ("Polar Coding Simulation" CXX)

#set(CMAKE_CXX_STANDARD 14)

#select the release build type by default to get optimization flags
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

#make sure our local CMake Modules path comes first
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)

find_package(CppUnit)
#find_package(Threads)
find_package(Doxygen)

SET(BUILD_SHARED_LIBS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
include_directories(${CMAKE_SOURCE_DIR}/include)
include(CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX("tclap/CmdLine.h" HAVE_TCLAP)
IF(NOT HAVE_TCLAP)
  message( FATAL_ERROR "TCLAP is not found (apt install libtclap-dev)" )
ENDIF()



add_definitions(-Wall -Wno-ignored-attributes)
add_definitions(-march=native -fPIC)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-O3)
else()
    add_definitions(-Og)
endif()

#add_definitions(-Wl,--no-as-needed)

add_subdirectory(src)
add_subdirectory(test)
#add_subdirectory(doc)
add_subdirectory(include/polarcode)


########################################################################
# Create uninstall target
########################################################################
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
@ONLY)

add_custom_target(uninstall
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)

########################################################################
# Setup the package config file
########################################################################
#set variables found in the pc.in file
set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/lib${LIB_SUFFIX}")
set(includedir "\${prefix}/include")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/PolarCode.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/PolarCode.pc
@ONLY)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/PolarCode.pc
    DESTINATION lib${LIB_SUFFIX}/pkgconfig
    COMPONENT "PolarCode_devel"
)

########################################################################
# Install cmake search helper for this library
########################################################################
if(NOT CMAKE_MODULES_DIR)
  set(CMAKE_MODULES_DIR lib/cmake)
endif(NOT CMAKE_MODULES_DIR)

install(FILES cmake/Modules/PolarCodeConfig.cmake
    DESTINATION ${CMAKE_MODULES_DIR}/PolarCode
)
