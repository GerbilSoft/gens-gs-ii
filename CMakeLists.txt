PROJECT(gens-gs-ii)

# Version dependencies:
# - BREAK(): CMake 2.6.0
# - NASM: CMake 2.8.4
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)

# Set default build options.
INCLUDE(cmake/options.cmake)

list (APPEND CMAKE_MODULE_PATH
		"${gens-gs-ii_SOURCE_DIR}/cmake/macros"
		"${gens-gs-ii_SOURCE_DIR}/cmake/libs"
		)

# Mac OS X: CMake doesn't include this by default.
INCLUDE(CheckLibraryExists)

# Split Debug macro.
MACRO(DO_SPLIT_DEBUG _TARGET)
	# Split debug information.
	INCLUDE(SetMSVCDebugPath)
	SET_MSVC_DEBUG_PATH(${_TARGET})
	IF(SPLIT_DEBUG)
		INCLUDE(SplitDebugInformation)
		SPLIT_DEBUG_INFORMATION(${_TARGET})
	ENDIF(SPLIT_DEBUG)
ENDMACRO(DO_SPLIT_DEBUG)

# Testing functions.
INCLUDE(cmake/gtest.cmake)

# Program information.
SET(DESCRIPTION "Gens/GS II")
SET(AUTHOR "David Korth")
SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 0)
SET(VERSION_PATCH 0)
SET(VERSION_DEVEL 1)
IF(VERSION_PATCH)
	SET(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
ELSE(VERSION_PATCH)
	SET(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}")
ENDIF(VERSION_PATCH)
IF(VERSION_DEVEL)
	SET(VERSION_STRING "${VERSION_STRING}+")
ENDIF(VERSION_DEVEL)
SET(VERSION_STRING_WIN32 "${VERSION_MAJOR},${VERSION_MINOR},${VERSION_PATCH},${VERSION_DEVEL}")

# Git version information.
FIND_PROGRAM(POSIX_SH sh)
IF(POSIX_SH)
	# sh is available.
	# Run the git version script.
	IF(WIN32)
		SET(ENV{SHELLOPTS} igncr)
	ENDIF(WIN32)
	ADD_CUSTOM_TARGET(git_version ALL
		${POSIX_SH} ${gens-gs-ii_SOURCE_DIR}/git_version.sh
		-s "${gens-gs-ii_SOURCE_DIR}"
		-o "${gens-gs-ii_BINARY_DIR}/git_version.h"
		VERBATIM
		)
ELSE(POSIX_SH)
	# sh isn't available.
	# Create a blank git_version.h.
	FILE(WRITE "${gens-gs-ii_BINARY_DIR}/git_version.h"
		"/* dummy file; POSIX sh is not available */\n")
ENDIF(POSIX_SH)

# If no build type is set, default to "debug".
IF(CMAKE_BUILD_TYPE MATCHES ^$)
	SET(CMAKE_BUILD_TYPE "debug")
ENDIF()

# Don't bother with verbose makefiles for now.
# TODO: Make it an option?
#IF (CMAKE_BUILD_TYPE MATCHES ^debug$)
	#SET(CMAKE_VERBOSE_MAKEFILE ON)
#ENDIF()

# Check for platform-specific functionality.
INCLUDE(cmake/platform.cmake)

# Check for required libraries.
INCLUDE(CheckZLIB)
INCLUDE(CheckPNG)
INCLUDE(CheckLZMA)
INCLUDE(CheckOpenGL)
INCLUDE(CheckPopt)

# Project subdirectories.
ADD_SUBDIRECTORY(extlib)
ADD_SUBDIRECTORY(src)

# Documentation.
IF(BUILD_DOC)
	ADD_SUBDIRECTORY(doc)
ENDIF(BUILD_DOC)
