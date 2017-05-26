project ("Polar Coding Simulation")
cmake_minimum_required(VERSION 3.0)

set(CMAKE_BUILD_TYPE Release)

add_definitions(-std=c++11 -Wall -mavx -mrdrnd -mbmi)
add_executable (pcsim main ArrayFuncs PolarCode PolarCode.MultiPath DataPool crc8)
target_link_libraries(pcsim pthread)
