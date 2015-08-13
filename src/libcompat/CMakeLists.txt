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
ENDMACRO(CHECK_FUNCTION_EXISTS_OR_INLINE VARIABLE)
CHECK_FUNCTION_EXISTS_OR_INLINE(localtime_r "time.h" "time_t tm; localtime_r(&tm, NULL);" HAVE_LOCALTIME_R)
CHECK_FUNCTION_EXISTS(getpwuid_r HAVE_GETPWUID_R)

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.libcompat.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.libcompat.h")

# NOTE: Currently, libcompat has no source files
# other than W32U. Hence, we need to include a dummy
# source file on non-Windows platforms.
IF(NOT WIN32)
SET(libcompat_SRCS dummy.c)
ENDIF(NOT WIN32)

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
	)
ENDIF(WIN32)

######################
# Build the library. #
######################

ADD_LIBRARY(compat STATIC ${libcompat_SRCS} ${libcompat_H})
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(compat)

IF(WIN32)
	# Shell32.dll is required for W32U_shlobj.c, which calls these functions:
	# - SHGetSpecialFolderPath()
	TARGET_LINK_LIBRARIES(compat shell32)
ENDIF(WIN32)
