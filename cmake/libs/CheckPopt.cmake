# Check for popt.
# If popt isn't found, extlib/popt/ will be used instead.
IF(NOT DEFINED HAVE_POPT)

FIND_LIBRARY(POPT_LIBRARY popt)
IF(POPT_LIBRARY)
	# System popt was found.
	SET(POPT_FOUND 1)
ENDIF(POPT_LIBRARY)

IF(NOT POPT_FOUND)
	# System popt was not found.
	# Use the included copy of popt.
	SET(POPT_LIBRARY popt)
	SET(POPT_FOUND 1)
	SET(HAVE_POPT 1)
	SET(POPT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extlib/popt/")
	SET(USE_INTERNAL_POPT 1)
	MESSAGE(STATUS "popt library not found; using internal popt.")
ENDIF(NOT POPT_FOUND)

# popt is always available.
SET(HAVE_POPT 1)

ENDIF(NOT DEFINED HAVE_POPT)
