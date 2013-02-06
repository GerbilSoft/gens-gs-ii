PROJECT(gens-gs-ii)

# Version dependencies:
# - BREAK(): CMake 2.6.0
# - NASM: CMake 2.8.4
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)

# UNSET() was added in cmake-2.6.3.
# Define an equivalent macro for older versions.
MACRO(UNSET var)
	SET(${var})
ENDMACRO(UNSET)

list (APPEND CMAKE_MODULE_PATH "${gens-gs-ii_SOURCE_DIR}/cmake/macros")

# Mac OS X: CMake doesn't include this by default.
INCLUDE(CheckLibraryExists)

# Testing functions.
INCLUDE(cmake/gtest.cmake)

# Set default build options.
INCLUDE(cmake/options.cmake)

# Git version information.
ADD_CUSTOM_TARGET(git_version ALL
	${gens-gs-ii_SOURCE_DIR}/git_version.sh -s "${gens-gs-ii_SOURCE_DIR}" -o "${gens-gs-ii_BINARY_DIR}/git_version.h"
	VERBATIM
)

# If no build type is set, default to "debug".
IF(CMAKE_BUILD_TYPE MATCHES ^$)
	SET(CMAKE_BUILD_TYPE "debug")
ENDIF()

# Don't bother with verbose makefiles for now.
# TODO: Make it an option?
#IF (CMAKE_BUILD_TYPE MATCHES ^debug$)
#	SET(CMAKE_VERBOSE_MAKEFILE ON)
#ENDIF()

# Check for platform-specific functionality.
INCLUDE(cmake/platform.cmake)

# Check for required libraries.
INCLUDE(cmake/libs/CheckZLIB.cmake)
INCLUDE(cmake/libs/CheckLZMA.cmake)
INCLUDE(cmake/libs/CheckOpenGL.cmake)

# Project subdirectories.
ADD_SUBDIRECTORY(extlib)
ADD_SUBDIRECTORY(src)

# Documentation.
IF(BUILD_DOC)
	ADD_SUBDIRECTORY(doc)
ENDIF(BUILD_DOC)
