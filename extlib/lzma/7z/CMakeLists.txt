# LZMA SDK 15.14, ANSI-C version.
# http://www.7-zip.org/sdk.html

# 7z-specific code.

PROJECT(7z)
cmake_minimum_required(VERSION 2.6.0)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../lzmabase/")

# Sources.
SET(7z_SRCS
	7zAlloc.c	# Memory allocator
	7zArcIn.c
	7zBuf.c
	7zBuf2.c
	7zDec.c
	7zFile.c
	7zStream.c
	)
SET(7z_H
	7z.h
	7zAlloc.h
	7zBuf.h
	7zFile.h
	7zVersion.h
	)

######################
# Build the library. #
######################

ADD_LIBRARY(7z STATIC ${7z_SRCS} ${7z_H})
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(7z)

# Exclude from ALL and DEFAULT builds.
SET_TARGET_PROPERTIES(7z PROPERTIES
	EXCLUDE_FROM_ALL TRUE
	EXCLUDE_FROM_DEFAULT_BUILD TRUE
	)
