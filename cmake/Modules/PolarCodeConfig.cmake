INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_POLARCODE PolarCode)

FIND_PATH(
    POLARCODE_INCLUDE_DIRS
    NAMES polarcode/decoding/decoder.h
    HINTS $ENV{POLARCODE_DIR}/include
        ${PC_POLARCODE_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    POLARCODE_LIBRARIES
    NAMES PolarCode
    HINTS $ENV{TEST38_DIR}/lib
        ${PC_POLARCODE_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

message(STATUS "PolarCode Include: ${POLARCODE_INCLUDE_DIRS}")
message(STATUS "PolarCode Library: ${POLARCODE_LIBRARIES}")
#add_library(PolarCode INTERFACE IMPORTED)
#set_target_properties(PolarCode PROPERTIES
#    INTERFACE_INCLUDE_DIRECTORIES "${POLARCODE_INCLUDE_DIRS}"
#)
include("${CMAKE_CURRENT_LIST_DIR}/PolarCodeTargets.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(POLARCODE DEFAULT_MSG POLARCODE_LIBRARIES POLARCODE_INCLUDE_DIRS)
MARK_AS_ADVANCED(POLARCODE_LIBRARIES POLARCODE_INCLUDE_DIRS)
