# Check for zlib.
# If zlib isn't found, extlib/zlib/ will be used instead.
IF(NOT DEFINED HAVE_ZLIB)

IF(WIN32)
	MESSAGE(STATUS "Win32: using internal ZLIB")
ELSEIF(USE_INTERNAL_PNG)
	# Internal PNG is in use.
	# Always use internal ZLIB.
	MESSAGE(STATUS "Using internal ZLIB due to internal PNG dependency.")
ELSE()
	# Only search for zlib on non-Win32 platforms.
	# Use the built-in zlib on Win32.
	FIND_PACKAGE(ZLIB)
ENDIF()
SET(ZLIB_DEFINITIONS -DZLIB_CONST)
SET(HAVE_ZLIB 1)

IF(NOT ZLIB_FOUND)
	SET(ZLIB_LIBRARY zlibstatic)
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

# Check for system MiniZip. (1.1 or later)
# MiniZip 1.1 added 64-bit support, so we'll check for unzOpen2_64().
IF (WIN32)
	MESSAGE(STATUS "Win32: using internal MINIZIP")
ELSE(WIN32)
	CHECK_LIBRARY_EXISTS(minizip unzOpen2_64 "" MINIZIP_FOUND)
ENDIF(WIN32)
SET(HAVE_MINIZIP 1)

IF (MINIZIP_FOUND)
	# System MiniZip was found.
	SET(MINIZIP_LIBRARY minizip)

	# Set the MiniZip include directory to an empty string to prevent errors.
	SET(MINIZIP_INCLUDE_DIR "")
ELSE(MINIZIP_FOUND)
	# System MiniZip was not found.
	# Use internal MiniZip.
	SET(MINIZIP_LIBRARY minizip)
	SET(MINIZIP_FOUND 1)
	SET(HAVE_MINIZIP 1)
	SET(MINIZIP_INCLUDE_DIR
		"${CMAKE_CURRENT_SOURCE_DIR}/extlib/minizip/"
		"${CMAKE_CURRENT_BINARY_DIR}/extlib/minizip/"
		)
	SET(USE_INTERNAL_MINIZIP 1)
	IF(NOT WIN32)
		# TODO: Check MINIZIP library version.
		# If MINIZIP exists but it's too old, say so.
		MESSAGE(STATUS "MINIZIP library not found; using internal MINIZIP.")
	ENDIF(NOT WIN32)
ENDIF(MINIZIP_FOUND)
SET(MINIZIP_DEFINITIONS -DSTRICTZIPUNZIP)

ENDIF(NOT DEFINED HAVE_ZLIB)
