PROJECT(libcompat)
cmake_minimum_required(VERSION 2.6.0)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../")

# libcompat source and binary directories.
# Binary directory is needed for config.h.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# Library checks.
INCLUDE(CheckFunctionExists)
MACRO(CHECK_FUNCTION_EXISTS_OR_INLINE FUNCTION HEADER SAMPLE_CODE VARIABLE)
	# MinGW defines some reentrant functions as inline functions
	# that are actually wrappers around MSVC "secure" functions.
	# Check for the function as a regular function first, then check
	# if it's available in the specified header.
	IF(NOT DEFINED ${VARIABLE})

	CHECK_FUNCTION_EXISTS(${FUNCTION} ${VARIABLE})
	IF(NOT ${VARIABLE})
		# Function does not exist normally.
		# Check if it's an inline function in the specified header.
		MESSAGE(STATUS "Looking for ${FUNCTION} as an inline function")
		CHECK_C_SOURCE_COMPILES(
			"#define _POSIX_SOURCE
			#define _POSIX_C_SOURCE 1
			#include <${HEADER}>
			int main(void) { ${SAMPLE_CODE} }"
			${VARIABLE}_INLINE)
		IF(${VARIABLE}_INLINE)
			MESSAGE(STATUS "Looking for ${FUNCTION} as an inline function - found")
			SET(${VARIABLE} 1 CACHE INTERNAL "Have function ${FUNCTION} (inline)")
		ELSE(${VARIABLE}_INLINE)
			MESSAGE(STATUS "Looking for ${FUNCTION} as an inline function - not found")
			# ${VARIABLE} is already cached by CHECK_FUNCTION_EXISTS().
		ENDIF(${VARIABLE}_INLINE)
	ENDIF(NOT ${VARIABLE})

	ENDIF(NOT DEFINED ${VARIABLE})
ENDMACRO(CHECK_FUNCTION_EXISTS_OR_INLINE VARIABLE)
CHECK_FUNCTION_EXISTS_OR_INLINE(localtime_r "time.h" "time_t tm; localtime_r(&tm, NULL);" HAVE_LOCALTIME_R)
CHECK_FUNCTION_EXISTS(getpwuid_r HAVE_GETPWUID_R)
# NOTE: These should either both be present or both be absent.
# We'll test for both of them anyway.
CHECK_FUNCTION_EXISTS(_fseeki64 HAVE_FSEEKI64)
CHECK_FUNCTION_EXISTS(_ftelli64 HAVE_FTELLI64)

# Check for getopt.h, getopt(), and getopt_long().
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckSymbolExists)
CHECK_INCLUDE_FILES(getopt.h HAVE_GETOPT_H)
CHECK_SYMBOL_EXISTS(getopt "getopt.h" HAVE_GETOPT)
CHECK_SYMBOL_EXISTS(getopt_long "getopt.h" HAVE_GETOPT_LONG)
IF(HAVE_GETOPT_H AND HAVE_GETOPT AND HAVE_GETOPT_LONG)
	# System has getopt.h, getopt(), and getopt_long().
	UNSET(getopt_SRCS)
	UNSET(getopt_H)
ELSE()
	# System is missing getopt.h, getopt(), and/or getopt_long().
	# Use vlc_getopt() instead.
	SET(getopt_SRCS vlc_getopt/getopt.c)
	SET(getopt_H vlc_getopt/vlc_getopt.h)
ENDIF()

# aligned_malloc.h
# NOTE: MinGW-w64 only has _aligned_malloc().
# It doesn't even have inline function wrappers
# for the other functions.
CHECK_FUNCTION_EXISTS(_aligned_malloc HAVE_MSVC_ALIGNED_MALLOC)
CHECK_FUNCTION_EXISTS(aligned_alloc HAVE_ALIGNED_ALLOC)
CHECK_FUNCTION_EXISTS(posix_memalign HAVE_POSIX_MEMALIGN)
CHECK_FUNCTION_EXISTS(memalign HAVE_MEMALIGN)

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.libcompat.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.libcompat.h")

# Determine the byteswap source file to use.
IF(NOT DEFINED libcompat_ARCH_SPECIFIC_SRCS)
	IF(NOT CMAKE_SYSTEM_PROCESSOR)
		MESSAGE(FATAL_ERROR "CMAKE_SYSTEM_PROCESSOR is empty.")
	ENDIF(NOT CMAKE_SYSTEM_PROCESSOR)
	STRING(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" arch)
	IF("${arch}" MATCHES i[0-9]86|x86|x86_64|amd64)
		SET(libcompat_ARCH_SPECIFIC_SRCS
			cpuflags_x86.c
			byteswap_x86.c
			)
	ELSE()
		SET(libcompat_ARCH_SPECIFIC_SRCS
			cpuflags.c
			byteswap.c
			)
	ENDIF()
	UNSET(arch)
ENDIF(NOT DEFINED libcompat_ARCH_SPECIFIC_SRCS)

SET(libcompat_SRCS
	${libcompat_ARCH_SPECIFIC_SRCS}
	)

SET(libcompat_H
	reentrant.h
	aligned_malloc.h
	cpuflags.h
	cpuflags_x86.h
	byteswap.h
	)

IF(WIN32)
# W32U sources.
SET(libcompat_SRCS
	${libcompat_SRCS}
	W32U/W32U_mini.c
	W32U/W32U_libc.c
	W32U/W32U_argv.c
	W32U/W32U_shlobj.c
	)

# W32U headers.
SET(libcompat_H
	${libcompat_H}
	W32U/W32U_mini.h
	W32U/W32U_libc.h
	W32U/W32U_argv.h
	W32U/W32U_shlobj.h
	W32U/is_unicode.h
	W32U/W32U_alloca.h
	)
ENDIF(WIN32)

######################
# Build the library. #
######################

ADD_LIBRARY(compat STATIC
	${libcompat_SRCS}
	${libcompat_H}
	${getopt_SRCS}
	${getopt_H}
	)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(compat)

IF(WIN32)
	# Shell32.dll is required for W32U_shlobj.c, which calls these functions:
	# - SHGetSpecialFolderPath()
	TARGET_LINK_LIBRARIES(compat shell32)
ENDIF(WIN32)

# Test suite.
IF(BUILD_TESTING)
       ADD_SUBDIRECTORY(tests)
ENDIF(BUILD_TESTING)
