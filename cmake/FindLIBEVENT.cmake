# -*- cmake -*-

# - Find BerkeleyDB
# Find the BerkeleyDB includes and library
# This module defines
#  EV_INCLUDE_DIR, where to find db.h, etc.
#  EV_LIBRARIES, the libraries needed to use BerkeleyDB.
#  EV_FOUND, If false, do not try to use BerkeleyDB.
# also defined, but not for general use are
#  EV_LIBRARY, where to find the BerkeleyDB library.

FIND_PATH(EV_INCLUDE_DIR event2/event.h
  /usr/local/include
  /usr/include
)

SET(EV_NAMES ${EV_NAMES} event event_extra )
FIND_LIBRARY(EV_LIBRARY
  NAMES ${EV_NAMES}
  PATHS /usr/lib /usr/local/lib
  )

FIND_LIBRARY(EV_LIBRARY_PTHREAD
  NAMES event_pthreads
  PATHS /usr/lib /usr/local/lib
  )
IF (EV_LIBRARY AND EV_INCLUDE_DIR)
  SET(EV_LIBRARIES ${EV_LIBRARY} ${EV_LIBRARY_PTHREAD})
  SET(EV_FOUND "YES")
ELSE (EV_LIBRARY AND EV_INCLUDE_DIR)
  SET(EV_FOUND "NO")
ENDIF (EV_LIBRARY AND EV_INCLUDE_DIR)


IF (EV_FOUND)
  IF (NOT EV_FIND_QUIETLY)
    MESSAGE(STATUS "Found LibEvent: ${EV_LIBRARIES}")
  ENDIF (NOT EV_FIND_QUIETLY)
ELSE (EV_FOUND)
  IF (EV_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find BerkeleyDB library")
  ENDIF (EV_FIND_REQUIRED)
ENDIF (EV_FOUND)

# Deprecated declarations.
SET (NATIVE_EV_INCLUDE_PATH ${EV_INCLUDE_DIR} )
GET_FILENAME_COMPONENT (NATIVE_EV_LIB_PATH ${EV_LIBRARY} PATH)

MARK_AS_ADVANCED(
  EV_LIBRARY
  EV_INCLUDE_DIR
  )
