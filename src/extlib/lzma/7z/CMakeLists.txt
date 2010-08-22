# LZMA SDK 9.12, ANSI-C version.
# http://www.7-zip.org/sdk.html

# 7z-specific code.

PROJECT(7z)
cmake_minimum_required(VERSION 2.6.0)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../lzmabase/")

# Sources.
SET(7z_SRCS
	7zBuf2.c
	7zBuf.c
	7zDec.c
	7zFile.c
	7zIn.c
	7zStream.c
	
	# 7-Zip memory allocator.
	7zAlloc.c
	)

# Build the library.
ADD_LIBRARY(7z STATIC ${7z_SRCS})
