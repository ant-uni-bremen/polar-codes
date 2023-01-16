# Copyright 2018, 2019, 2020 Florian Lotze, Johannes Demel
#
# This file is part of PolarCodes
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 3.16)
project ("polar-codes" VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.20)
    cmake_policy(SET CMP0115 OLD)
endif()

#select the release build type by default to get optimization flags
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#make sure our local CMake Modules path comes first
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)

find_package(CppUnit REQUIRED)
if(NOT CPPUNIT_FOUND)
	message(FATAL_ERROR "CppUnit missing! (apt install libcppunit-dev)")
endif()
#find_package(Threads)
find_package(Doxygen)
find_package(OpenSSL REQUIRED)
#find_package(cppunit)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(fmt REQUIRED)
message(STATUS "fmt version: ${fmt_VERSION}")

## Check for TCLAP!
set(CMAKE_REQUIRED_INCLUDES "${CMAKE_INSTALL_PREFIX}/include")
include(CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX("tclap/CmdLine.h" HAVE_TCLAP)
IF(NOT HAVE_TCLAP)
  message( FATAL_ERROR "TCLAP is not found (apt install libtclap-dev)" )
ENDIF()

# Probably the preferred way
find_package(pybind11)
if(NOT pybind11_FOUND)
    add_subdirectory(pybind11)
endif()

SET(BUILD_SHARED_LIBS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# We need to do a CMAKE_INSTALL_PREFIX include. Otherwise, we can't make the compiler happy in a conda env!
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(SYSTEM ${CMAKE_INSTALL_PREFIX}/include)


add_definitions(-Wall -Wno-ignored-attributes)
add_definitions(-march=native -fPIC -mavx2)
#add_definitions(-funroll-loops)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-O3)
else()
    add_definitions(-Og)
endif()

#add_definitions(-Wl,--no-as-needed)

enable_testing()


########################################################################
# Set library install dir!
########################################################################
# See:https://gitlab.kitware.com/cmake/cmake/blob/18365587c86396f988e256b5acf4d2312f3be2bb/Modules/GNUInstallDirs.cmake#L210-212

include(GNUInstallDirs)

set(INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
set(IS_DEBIANOID_SYSTEM False)
if (EXISTS "/etc/debian_version")
    set(IS_DEBIANOID_SYSTEM True)
    if("${CMAKE_INSTALL_PREFIX}" MATCHES "^/usr/local/?$")
        set(INSTALL_LIBDIR "lib/${CMAKE_LIBRARY_ARCHITECTURE}")
    endif()
endif()
set(INSTALL_FULL_LIBDIR "${CMAKE_INSTALL_PREFIX}/${INSTALL_LIBDIR}")


########################################################################
# Sets the python installation directory PYTHON_DIR
########################################################################
message(STATUS "Py3k Executable: ${Python3_EXECUTABLE}")
message(STATUS "Py3k STDLIB: ${Python3_STDLIB}")
message(STATUS "Py3k STDARCH: ${Python3_STDARCH}")
message(STATUS "Py3k SITELIB: ${Python3_SITELIB}")
message(STATUS "Py3k SITEARCH: ${Python3_SITEARCH}")

message(STATUS "Py3k NumPy FOUND: ${Python3_NumPy_FOUND}")
message(STATUS "Py3k NumPy VERSION: ${Python3_NumPy_VERSION}")

message(STATUS "NumPy: ${PYTHON_NUMPY_INCLUDE_DIR}")

if(NOT DEFINED PYTHON_DIR)
execute_process(COMMAND ${Python3_EXECUTABLE} -c "import os
import sysconfig
import site
install_dir = None
# The next line passes a CMake variable into our script.
prefix = '${CMAKE_INSTALL_PREFIX}'
# We use `site` to identify if our chosen prefix is a default one.
# https://docs.python.org/3/library/site.html
try:
    # https://docs.python.org/3/library/site.html#site.getsitepackages
    paths = [p for p in site.getsitepackages() if p.startswith(prefix)]
    if len(paths) == 1: install_dir = paths[0]
except AttributeError: pass
# If we found a default install path, `install_dir` is set.
if not install_dir:
    # We use a custom install prefix!
    # Determine the correct install path in that prefix on the current platform.
    # For Python 3.11+, we could use the 'venv' scheme for all platforms
    # https://docs.python.org/3.11/library/sysconfig.html#installation-paths
    if os.name == 'nt':
        scheme = 'nt'
    else:
        scheme = 'posix_prefix'
    install_dir = sysconfig.get_path('platlib', scheme)
    prefix = sysconfig.get_path('data', scheme)
#strip the prefix to return a relative path
print(os.path.relpath(install_dir, prefix))"
    OUTPUT_VARIABLE PYTHON_DIR 
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
endif()
file(TO_CMAKE_PATH ${PYTHON_DIR} PYTHON_DIR)
message(STATUS "The PYTHON install dir: ${PYTHON_DIR}")

########################################################################
# Add subdirectories with actual code
########################################################################
add_subdirectory(src)
add_subdirectory(test)
#add_subdirectory(doc)
add_subdirectory(include/polarcode)
add_subdirectory(python)


########################################################################
# Include benchmark folder if Google benchmark is available
########################################################################
find_package(benchmark QUIET)
if(benchmark_FOUND)
    message(STATUS "Add benchmarks ...")
    add_subdirectory(benchmark)
else()
message(STATUS "Skipping benchmarks ...")
endif(benchmark_FOUND)


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
set(libdir "\${exec_prefix}/${INSTALL_LIBDIR}")
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

message(STATUS "CMake Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Using install libdir: ${INSTALL_FULL_LIBDIR}")
