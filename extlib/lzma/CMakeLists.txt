# LZMA SDK 15.14, ANSI-C version.
# http://www.7-zip.org/sdk.html
PROJECT(lzma)
cmake_minimum_required(VERSION 2.6.0)

# Sources.
SET(lzma_SRCS
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
SET(lzma_H
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

# 7z archive support.
SET(lzma_7z_SRCS
	7zAlloc.c	# Memory allocator
	7zArcIn.c
	7zBuf.c
	7zBuf2.c
	7zDec.c
	7zFile.c
	7zStream.c
	)
SET(lzma_7z_H
	7z.h
	7zAlloc.h
	7zBuf.h
	7zFile.h
	7zVersion.h
	)

### Optional components. ###

# Encoding.
SET(lzma_ENCODE_SRCS
	Bcj2Enc.c
	Lzma2Enc.c
	Lzma86Enc.c
	LzmaEnc.c
	Ppmd7Enc.c
	XzEnc.c
	)
SET(lzma_ENCODE_H
	Lzma2Enc.h
	LzmaEnc.h
	XzEnc.h
	)

# Multithreading.
SET(lzma_MT_SRCS
	LzFindMt.c
	MtCoder.c
	Threads.c
	)
SET(lzma_MT_H
	LzFindMt.h
	MtCoder.h
	Threads.h
	)

# Encryption.
SET(lzma_CRYPTO_SRCS
	Aes.c
	AesOpt.c	# MSVC only.
	)
SET(lzma_CRYPTO_H
	Aes.h
	)

# LzmaLib.
SET(lzma_LzmaLib_SRCS
	LzmaLib.c
	)
SET(lzma_LzmaLib_H
	LzmaLib.h
	)

# Sort.
SET(lzma_SORT_SRCS
	Sort.c
	)
SET(lzma_SORT_H
	Sort.h
	)

######################
# Build the library. #
######################

ADD_LIBRARY(lzma STATIC
	${lzma_SRCS} ${lzma_H}
	${lzma_7z_SRCS} ${lzma_7z_H}
	)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(lzma)

# Exclude from ALL and DEFAULT builds.
SET_TARGET_PROPERTIES(lzma PROPERTIES
	EXCLUDE_FROM_ALL TRUE
	EXCLUDE_FROM_DEFAULT_BUILD TRUE
	)
