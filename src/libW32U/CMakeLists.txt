PROJECT(libW32U)
cmake_minimum_required(VERSION 2.6.0)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../")

# Sources.
SET(libW32U_SRCS
	W32U_mini.c
	W32U_libc.c
	W32U_argv.c
	)

# Headers.
SET(libW32U_H
	W32U_mini.h
	W32U_libc.h
	W32U_argv.h
	is_unicode.h
	)

######################
# Build the library. #
######################

ADD_LIBRARY(W32U STATIC
	${libW32U_SRCS} ${libW32U_H}
	)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(W32U)
