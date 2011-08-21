# Check for zlib.
# If zlib isn't found, extlib/zlib/ will be used instead.
IF(WIN32)
	MESSAGE(STATUS "Win32: using internal ZLIB")
ELSE(WIN32)
	# Only search for zlib on non-Win32 platforms.
	# Use the built-in zlib on Win32.
	FIND_PACKAGE(ZLIB)
ENDIF(WIN32)
SET(HAVE_ZLIB 1)

IF(NOT ZLIB_FOUND)
	SET(ZLIB_LIBRARY zlib)
	SET(ZLIB_FOUND 1)
	SET(HAVE_ZLIB 1)
	SET(ZLIB_INCLUDE_DIR
		"${CMAKE_CURRENT_SOURCE_DIR}/extlib/zlib/"
		"${CMAKE_CURRENT_BINARY_DIR}/extlib/zlib/"
		)
	SET(USE_INTERNAL_ZLIB 1)
	IF(NOT WIN32)
		MESSAGE(STATUS "ZLIB library not found; using internal ZLIB.")
	ENDIF(NOT WIN32)
ENDIF(NOT ZLIB_FOUND)
