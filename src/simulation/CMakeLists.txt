add_executable (pcsim main setup simulator statistics)
target_link_libraries(pcsim pthread PolarCode SignalProcessing)
