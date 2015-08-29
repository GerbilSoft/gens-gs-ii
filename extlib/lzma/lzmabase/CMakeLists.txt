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

ADD_LIBRARY(lzmabase STATIC ${lzmabase_SRCS})
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(lzmabase)

# Exclude from ALL and DEFAULT builds.
SET_TARGET_PROPERTIES(lzmabase PROPERTIES
	EXCLUDE_FROM_ALL TRUE
	EXCLUDE_FROM_DEFAULT_BUILD TRUE
	)
