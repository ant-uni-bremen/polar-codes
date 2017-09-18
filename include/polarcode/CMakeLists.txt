
########################################################################
# Install public header files
########################################################################
add_subdirectory(encoding)
add_subdirectory(decoding)
add_subdirectory(errordetection)

install(FILES
    bitcontainer.h DESTINATION include/polarcode
)

