# UnRAR.dll (libgensunrar.so)
# Copyright © Alexander Roshal 1993-2016
# CMakeLists.txt for Gens/GS II.
PROJECT(unrar)
cmake_minimum_required(VERSION 2.4.4)

# UnRAR.dll sources.
SET(unrar_SRCS
	archive.cpp
	arcread.cpp
	blake2s.cpp
	cmddata.cpp	# TODO: May not be needed?
	consio.cpp
	crc.cpp
	dll.cpp
	encname.cpp
	errhnd.cpp
	extinfo.cpp
	extract.cpp
	filcreat.cpp
	file.cpp
	filefn.cpp
	filestr.cpp
	find.cpp
	getbits.cpp
	global.cpp
	hash.cpp
	headers.cpp
	isnt.cpp	# Windows only.
	match.cpp
	options.cpp
	pathfn.cpp
	qopen.cpp
	rar.cpp
	rarvm.cpp
	rawread.cpp
	rdwrfn.cpp
	rijndael.cpp
	rs.cpp
	rs16.cpp
	scantree.cpp
	secpassword.cpp
	sha1.cpp
	sha256.cpp
	smallfn.cpp
	strfn.cpp
	strlist.cpp
	system.cpp
	threadpool.cpp
	timefn.cpp
	ui.cpp
	unicode.cpp
	unpack.cpp
	volume.cpp
	)
SET(unrar_H
	archive.hpp
	blake2s.hpp
	cmddata.hpp	# TODO: May not be needed?
	consio.hpp
	crc.hpp
	dll.hpp
	encname.hpp
	errhnd.hpp
	extinfo.hpp
	extract.hpp
	filcreat.hpp
	file.hpp
	filefn.hpp
	filestr.hpp
	find.hpp
	getbits.hpp
	global.hpp
	hash.hpp
	headers.hpp
	isnt.hpp	# Windows only.
	match.hpp
	options.hpp
	pathfn.hpp
	qopen.hpp
	rar.hpp
	rarvm.hpp
	rawread.hpp
	rdwrfn.hpp
	rijndael.hpp
	rs.hpp
	rs16.hpp
	scantree.hpp
	secpassword.hpp
	sha1.hpp
	sha256.hpp
	smallfn.hpp
	strfn.hpp
	strlist.hpp
	system.hpp
	threadpool.hpp
	timefn.hpp
	ui.hpp
	unicode.hpp
	unpack.hpp
	volume.hpp
	)

# Encryption code.
SET(unrar_CRYPT_SRCS
	crypt.cpp
	)
SET(unrar_CRYPT_H
	crypt.hpp
	)

# Add some compiler flags to suppress lots of warnings.
IF(MSVC)
	# Ignore macro redefinitions.
	# Macros defined in CMake that are also defined in raros.hpp:
	# - STRICT, RAR_SMP, WIN32_LEAN_AND_MEAN, USE_SSE.
	SET(UNRAR_WARNING_FLAGS "-wd4005")
ELSE(MSVC) # TODO: GCC?
	# TODO: Ensure "-fvisibility=hidden" is specified.
	# Strip -Wall -Wextra.
	STRING(REPLACE "-Wall"   "" CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}")
	STRING(REPLACE "-Wall"   "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	STRING(REPLACE "-Wextra" "" CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}")
	STRING(REPLACE "-Wextra" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

	# UnRAR.dll uses exceptions, so we can't disable them.
	# TODO: Use CheckCXXNoExceptionsCompilerFlag()?
	# (though that has "-EHsc" for MSVC, which is acceptable...)
	STRING(REPLACE "-fno-exceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
ENDIF(MSVC)

IF(UNRAR_WARNING_FLAGS)
	SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${UNRAR_WARNING_FLAGS}")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${UNRAR_WARNING_FLAGS}")
ENDIF(UNRAR_WARNING_FLAGS)

# Required compile definitions.
ADD_DEFINITIONS(-DRARDLL -DUNRAR -DSILENT)

# TODO: Make these options.
SET(RAR_USE_SMP 1)
SET(RAR_USE_SSE 1)

IF(RAR_USE_SMP)
	# Multi-threaded extraction.
	ADD_DEFINITIONS(-DRAR_SMP)
ENDIF(RAR_USE_SMP)

IF(MSVC AND RAR_USE_SSE)
	# SSE optimizations.
	# TODO: Port SSE optimizations to gcc.
	STRING(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" arch)
	IF(arch MATCHES "^(i.|x)86$|^x86_64$|^amd64$")
		ADD_DEFINITIONS(-DUSE_SSE)
	ENDIF()
	UNSET(arch)
ENDIF(MSVC AND RAR_USE_SSE)

IF(WIN32)
	# Win32 resources.
	SET(unrar_PLATFORM_SRCS dll.rc)
ENDIF(WIN32)

# TODO: Generate a PCH file using rarpch.cpp.

######################
# Build the library. #
######################

ADD_LIBRARY(gensunrar SHARED
	${unrar_SRCS} ${unrar_H}
	${unrar_CRYPT_SRCS} ${unrar_CRYPT_H}
	${unrar_PLATFORM_SRCS}
	)
INCLUDE(SetMSVCDebugPath)
DO_SPLIT_DEBUG(gensunrar)

# DLL version.
# NOTE: The 32-bit build is 100% backwards-compatible.
# The 64-bit build has an ABI breakage with v5, though
# 64-bit v4 DLLs are probably rare.
SET_TARGET_PROPERTIES(gensunrar PROPERTIES
	VERSION 0.7	# RAR_DLL_VERSION
	SOVERSION 0	# No ABI breakage.
	)

IF(WIN32)
	# .DEF file for exports.
	SET_TARGET_PROPERTIES(gensunrar
		PROPERTIES LINK_FLAGS "-DEF:${CMAKE_CURRENT_SOURCE_DIR}/dll.def"
		)
ENDIF(WIN32)
