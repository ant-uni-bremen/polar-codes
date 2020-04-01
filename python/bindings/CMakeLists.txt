########################################################################
# Python Bindings
########################################################################

pybind11_add_module(polarcode_python
    puncturer_python.cc
    encoder_python.cc
    decoder_python.cc
    detector_python.cc
    frozen_bits_python.cc
    python_bindings.cc
)

target_include_directories(polarcode_python PUBLIC
    ${PYTHON_NUMPY_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${PYBIND11_INCLUDE_DIR}
)
target_link_libraries(polarcode_python PUBLIC ${PYTHON_LIBRARIES} PolarCode)

if(CMAKE_COMPILER_IS_GNUCC AND NOT APPLE)
    SET_TARGET_PROPERTIES(polarcode_python
    PROPERTIES LINK_FLAGS "-Wl,--no-as-needed")
endif()

install(TARGETS polarcode_python DESTINATION ${PYTHON_DIR}/pypolar COMPONENT pythonapi)
