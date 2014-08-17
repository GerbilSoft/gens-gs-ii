# LZMA SDK 9.20, ANSI-C version.
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

######################
# Build the library. #
######################

SET(7z_PDB_FILE "${CMAKE_CURRENT_BINARY_DIR}/7z.pdb")

IF(MSVC)
	# CMake seems to use weird settings for the PDB file.
	# (at least version 2.8.12.2; 3.0.0 might be different)
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_C}"
		"/Fd${7z_PDB_FILE}"
		CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT}")
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_CXX}"
		"/Fd${7z_PDB_FILE}"
		CMAKE_CXX_COMPILE_OBJECT "${CMAKE_CXX_COMPILE_OBJECT}")
ENDIF(MSVC)

# Build the library.
ADD_LIBRARY(7z STATIC ${7z_SRCS})
