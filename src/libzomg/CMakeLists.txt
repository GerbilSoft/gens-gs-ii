PROJECT(libzomg)
cmake_minimum_required(VERSION 2.6.0)

# LibGens source and binary directories.
# Binary directory is needed for config.h.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# ZLIB include directory.
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

# Library checks.
INCLUDE(CheckLibraryExists)
CHECK_LIBRARY_EXISTS(c localtime_r "" HAVE_LOCALTIME_R)	# libc: localtime_r()

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h")

# Sources.
SET(libzomg_SRCS
	Zomg.cpp
	ZomgLoad.cpp
	ZomgSave.cpp
	zomg_byteswap.c
	)

# Build the library.
ADD_LIBRARY(zomg STATIC ${libzomg_SRCS})
TARGET_LINK_LIBRARIES(zomg minizip)
ADD_DEPENDENCIES(zomg minizip)
