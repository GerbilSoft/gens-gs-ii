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
# NOTE: May be _localtime32_s() or _localtime64_s() on MSVC 2005+.
# The "inline" part will detect that.
# FIXME: CHECK_SYMBOL_EXISTS() works for macros like in MSVC,
# but fails with inline functions like in MinGW-w64.
CHECK_FUNCTION_EXISTS_OR_INLINE(localtime_s "time.h" "time_t tm; localtime_s(&tm, NULL);" HAVE_LOCALTIME_S)
CHECK_FUNCTION_EXISTS(getpwuid_r HAVE_GETPWUID_R)

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
	IF(arch MATCHES "^(i.|x)86$|^x86_64$|^amd64$")
		SET(libcompat_ARCH_SPECIFIC_SRCS
			cpuflags_x86.c
			byteswap_x86.c
			)
	ELSEIF(arch MATCHES "powerpc|powerpc-eabi")
		SET(libcompat_ARCH_SPECIFIC_SRCS
			cpuflags_ppc.c
			byteswap.c
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

######################
# Build the library. #
######################

ADD_LIBRARY(compat STATIC
	${libcompat_SRCS}
	${libcompat_H}
	)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(compat)

# Test suite.
IF(BUILD_TESTING)
       ADD_SUBDIRECTORY(tests)
ENDIF(BUILD_TESTING)

# Win32 Unicode Translation Layer. (Mini Version)
IF(WIN32)
	ADD_SUBDIRECTORY(W32U)
ENDIF(WIN32)
