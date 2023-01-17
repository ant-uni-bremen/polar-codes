#
# Find the TCLAP includes
#
# This module defines
# TCLAP_INCLUDE_DIR, where to find CmdLine.h, etc.
# TCLAP_FOUND, If false, do not try to use CppUnit.


SET(TCLAP_FOUND "NO")

FIND_PATH(TCLAP_INCLUDE_DIR NAMES tclap/CmdLine.h 
	HINTS ${CMAKE_INSTALL_PREFIX}/include 
	PATHS /usr/local/include /usr/include
)

IF(TCLAP_INCLUDE_DIR)
  SET(TCLAP_FOUND "YES")
ELSE(TCLAP_INCLUDE_DIR)
  IF (TCLAP_FIND_REQUIRED)
    MESSAGE(SEND_ERROR "Could not find library TCLAP.")
  ENDIF(TCLAP_FIND_REQUIRED)
ENDIF(TCLAP_INCLUDE_DIR)
