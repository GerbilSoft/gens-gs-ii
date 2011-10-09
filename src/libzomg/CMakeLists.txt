PROJECT(libzomg)
cmake_minimum_required(VERSION 2.6.0)

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../")

# LibGens source and binary directories.
# Binary directory is needed for config.h.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# ZLIB and MINIZIP include directories.
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${MINIZIP_INCLUDE_DIR})

# Library checks.
INCLUDE(CheckLibraryExists)
CHECK_LIBRARY_EXISTS(c localtime_r "" HAVE_LOCALTIME_R)	# libc: localtime_r()

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.libzomg.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.libzomg.h")

# Test system byteorder.
# TODO: Universal Binary support; detect PDP endian.
INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
IF(IS_BIG_ENDIAN)
	SET(ZOMG_BYTEORDER "ZOMG_BIG_ENDIAN")
ELSE(IS_BIG_ENDIAN)
	SET(ZOMG_BYTEORDER "ZOMG_LIL_ENDIAN")
ENDIF(IS_BIG_ENDIAN)
UNSET(IS_BIG_ENDIAN)
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/zomg_byteorder.h.in" "${CMAKE_CURRENT_BINARY_DIR}/zomg_byteorder.h")

# Sources.
SET(libzomg_SRCS
	Zomg.cpp
	ZomgLoad.cpp
	ZomgSave.cpp
	zomg_byteswap.c
	)

# Build the library.
ADD_LIBRARY(zomg STATIC ${libzomg_SRCS})
TARGET_LINK_LIBRARIES(zomg ${MINIZIP_LIBRARY})
