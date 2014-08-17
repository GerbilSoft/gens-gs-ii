# LZMA SDK 9.20, ANSI-C version.
# http://www.7-zip.org/sdk.html

# Base LZMA/XZ code.

PROJECT(lzmabase)
cmake_minimum_required(VERSION 2.6.0)

# Sources.
SET(lzmabase_SRCS
	7zCrc.c
	7zCrcOpt.c
	Alloc.c
	Bcj2.c
	Bra86.c
	Bra.c
	BraIA64.c
	CpuArch.c
	Delta.c
	LzFind.c
	Lzma2Dec.c
	Lzma86Dec.c
	LzmaDec.c
	Ppmd7.c
	Ppmd7Dec.c
	Ppmd8.c
	Ppmd8Dec.c
	Sha256.c
	Xz.c
	XzCrc64.c
	XzDec.c
	XzIn.c
	)

######################
# Build the library. #
######################

SET(LZMABASE_PDB_FILE "${CMAKE_CURRENT_BINARY_DIR}/lzmabase.pdb")

IF(MSVC)
	# CMake seems to use weird settings for the PDB file.
	# (at least version 2.8.12.2; 3.0.0 might be different)
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_C}"
		"/Fd${LZMABASE_PDB_FILE}"
		CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT}")
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_CXX}"
		"/Fd${LZMABASE_PDB_FILE}"
		CMAKE_CXX_COMPILE_OBJECT "${CMAKE_CXX_COMPILE_OBJECT}")
ENDIF(MSVC)

ADD_LIBRARY(lzmabase STATIC ${lzmabase_SRCS})
