# LZMA SDK 15.14, ANSI-C version.
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
	Bra.c
	Bra86.c
	BraIA64.c
	CpuArch.c
	Delta.c
	LzFind.c
	Lzma2Dec.c
	Lzma86Dec.c
	LzmaDec.c
	Ppmd7.c
	Ppmd7Dec.c
	Sha256.c
	Xz.c
	XzCrc64.c
	XzCrc64Opt.c
	XzDec.c
	XzIn.c
	)
SET(lzmabase_H
	7zCrc.h
	7zTypes.h
	Alloc.h
	Bcj2.h
	Bra.h
	Compiler.h
	CpuArch.h
	Delta.h
	LzFind.h
	LzHash.h
	Lzma2Dec.h
	Lzma86.h
	LzmaDec.h
	Ppmd.h
	Ppmd7.h
	Precomp.h
	RotateDefs.h
	Sha256.h
	Xz.h
	XzCrc64.h
	)

# Encoding.
SET(lzmabase_ENCODE_SRCS
	Bcj2Enc.c
	Lzma2Enc.c
	Lzma86Enc.c
	LzmaEnc.c
	Ppmd7Enc.c
	XzEnc.c
	)
SET(lzmabase_ENCODE_H
	Lzma2Enc.h
	LzmaEnc.h
	XzEnc.h
	)

# Multithreading.
SET(lzmabase_MT_SRCS
	LzFindMt.c
	MtCoder.c
	Threads.c
	)
SET(lzmabase_MT_H
	LzFindMt.h
	MtCoder.h
	Threads.h
	)

# Encryption.
SET(lzmabase_CRYPTO_SRCS
	Aes.c
	AesOpt.c	# MSVC only.
	)
SET(lzmabase_CRYPTO_H
	Aes.h
	)

# LzmaLib.
SET(lzmabase_LzmaLib_SRCS
	LzmaLib.c
	)
SET(lzmabase_LzmaLib_H
	LzmaLib.h
	)

# Sort.
SET(lzmabase_SORT_SRCS
	Sort.c
	)
SET(lzmabase_SORT_H
	Sort.h
	)

######################
# Build the library. #
######################

ADD_LIBRARY(lzmabase STATIC
	${lzmabase_SRCS}
	${lzmabase_H}
	)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(lzmabase)

# Exclude from ALL and DEFAULT builds.
SET_TARGET_PROPERTIES(lzmabase PROPERTIES
	EXCLUDE_FROM_ALL TRUE
	EXCLUDE_FROM_DEFAULT_BUILD TRUE
	)
