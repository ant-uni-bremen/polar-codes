add_executable (errorlocator main setup simulator statistics)
target_link_libraries(errorlocator pthread PolarCode SignalProcessing)
