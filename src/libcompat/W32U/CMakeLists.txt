PROJECT(libcompat_W32U)
cmake_minimum_required(VERSION 2.6.0)

IF(NOT WIN32)
MESSAGE(FATAL_ERROR "libcompat/W32U should only be built for Windows.")
ENDIF(NOT WIN32)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the src/ directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../../")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../../")

# libcompat source and binary directories.
# Binary directory is needed for config.h.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../")

# libcompat/W32U source and binary directories.
# Binary directory is needed for config.h.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# Library checks.
INCLUDE(CheckFunctionExists)
# NOTE: These should either both be present or both be absent.
# We'll test for both of them anyway.
CHECK_FUNCTION_EXISTS(_stati64 HAVE__STATI64)
CHECK_FUNCTION_EXISTS(_fseeki64 HAVE__FSEEKI64)
CHECK_FUNCTION_EXISTS(_ftelli64 HAVE__FTELLI64)

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.libcompat_W32U.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.libcompat_W32U.h")

SET(libcompat_W32U_SRCS
	W32U_mini.c
	W32U_libc.c
	W32U_argv.c
	W32U_shlobj.c
	)
SET(libcompat_W32U_H
	W32U_mini.h
	W32U_libc.h
	W32U_argv.h
	W32U_shlobj.h
	is_unicode.h
	W32U_alloca.h
	)

######################
# Build the library. #
######################

ADD_LIBRARY(compat_W32U STATIC
	${libcompat_W32U_SRCS}
	${libcompat_W32U_H}
	)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(compat_W32U)

# Shell32.dll is required for W32U_shlobj.c, which calls these functions:
# - SHGetSpecialFolderPath()
TARGET_LINK_LIBRARIES(compat_W32U shell32)

# MiniZip UTF-8 compatibility for Windows.
IF(HAVE_MINIZIP)
	INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
	INCLUDE_DIRECTORIES(${MINIZIP_INCLUDE_DIR})
	SET(libcompat_W32U_minizip_SRCS minizip_iowin32u.c)
	SET(libcompat_W32U_minizip_H minizip_iowin32u.h)
	ADD_LIBRARY(compat_W32U_minizip STATIC
		${libcompat_W32U_minizip_SRCS}
		${libcompat_W32U_minizip_H}
		)
	SET_MSVC_DEBUG_PATH(compat_W32U_minizip)
	TARGET_LINK_LIBRARIES(compat_W32U_minizip compat_W32U ${MINIZIP_LIBRARY})
ENDIF(HAVE_MINIZIP)

# Test suite.
IF(BUILD_TESTING)
       ADD_SUBDIRECTORY(tests)
ENDIF(BUILD_TESTING)
