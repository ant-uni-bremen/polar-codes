project ("Polar Coding Simulation")
cmake_minimum_required(VERSION 3.0)

set(CMAKE_BUILD_TYPE Release)

add_definitions(-std=c++11 -mavx -mrdrnd)
add_executable (pcsim main ArrayFuncs PolarCode PolarCode.MultiPath crc8)
target_link_libraries(pcsim pthread)
