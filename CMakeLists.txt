project ("Polar Coding Simulation")
cmake_minimum_required(VERSION 2.8)
find_package(Doxygen)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")
find_package(CppUnit)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_definitions(-std=c++11 -Wall)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(doc)
