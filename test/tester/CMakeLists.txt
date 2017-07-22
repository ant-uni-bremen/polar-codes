add_executable(pctest
    tester
    $<TARGET_OBJECTS:PolarTest>)

#ADD_CUSTOM_COMMAND(TARGET pctest POST_BUILD
#  COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pctest")
ADD_CUSTOM_TARGET(tests "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pctest"
    DEPENDS pctest PolarCode PolarTest
    COMMENT "Running CPPUNIT tests...")

target_link_libraries(pctest PolarCode cppunit)
