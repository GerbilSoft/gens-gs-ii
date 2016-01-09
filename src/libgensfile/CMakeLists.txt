# libgensfile: Gens file handling library.
# This library is used for decompressing various archive formats.
PROJECT(libgensfile)
cmake_minimum_required(VERSION 2.6.0)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../")

# LibGensFile source and binary directories.
# Binary directory is needed for config.h.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# zlib and MiniZip include directories.
IF(HAVE_ZLIB)
	INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
	IF(HAVE_MINIZIP)
		INCLUDE_DIRECTORIES(${MINIZIP_INCLUDE_DIR})
	ENDIF(HAVE_MINIZIP)
ENDIF(HAVE_ZLIB)

# LZMA include directory.
IF(HAVE_LZMA)
	INCLUDE_DIRECTORIES(${LZMA_INCLUDE_DIR})
ENDIF(HAVE_LZMA)

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.libgensfile.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.libgensfile.h")

# Sources.
SET(libgensfile_SRCS
	Archive.cpp
	ArchiveFactory.cpp
	MemFake.cpp
	)
SET(libgensfile_H
	Archive.hpp
	ArchiveFactory.hpp
	MemFake.hpp
	)

# Gzip, Zip (via zlib)
IF(HAVE_ZLIB)
	SET(libgensfile_SRCS ${libgensfile_SRCS} Gzip.cpp)
	SET(libgensfile_H    ${libgensfile_H}    Gzip.hpp)
	IF(HAVE_MINIZIP)
		SET(libgensfile_SRCS ${libgensfile_SRCS} Zip.cpp)
		SET(libgensfile_H    ${libgensfile_H}    Zip.hpp)
	ENDIF(HAVE_MINIZIP)
ENDIF(HAVE_ZLIB)

# LZMA (7-Zip)
IF(HAVE_LZMA)
	# LZMA include directory.
	INCLUDE_DIRECTORIES(${LZMA_INCLUDE_DIR})

	# 7-Zip archive handler.
	SET(libgensfile_SRCS
		${libgensfile_SRCS}
		LzmaSdk.cpp
		Sz.cpp
		Xz.cpp
		Lzma.cpp
		)
	SET(libgensfile_H
		${libgensfile_H}
		LzmaSdk.hpp
		Sz.hpp
		Xz.hpp
		Lzma.hpp
		)
ENDIF(HAVE_LZMA)

# RAR (UnRAR.dll)
# TODO: Add an option to disable RAR support?
SET(libgensfile_SRCS
	${libgensfile_SRCS}
	UnRAR_dll.cpp
	Rar.cpp
	)
SET(libgensfile_H
	${libgensfile_H}
	UnRAR_dll.hpp
	Rar.hpp
	)

######################
# Build the library. #
######################

ADD_LIBRARY(gensfile STATIC
	${libgensfile_SRCS}
	${libgensfile_H}
	)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(gensfile)
TARGET_LINK_LIBRARIES(gensfile compat)

# TODO: Only link in libgenstext if HAVE_LZMA and HAVE_RAR.
TARGET_LINK_LIBRARIES(gensfile genstext)

# zlib and MiniZip
IF(HAVE_ZLIB)
	# NOTE: Due to linking order requirements, we have to link
	# MiniZip before zlib; otherwise, we might lose some symbols
	# in zlib.
	IF(HAVE_MINIZIP)
		TARGET_LINK_LIBRARIES(gensfile ${MINIZIP_LIBRARY})
	ENDIF(HAVE_MINIZIP)
	TARGET_LINK_LIBRARIES(gensfile ${ZLIB_LIBRARY} ${MINIZIP_LIBRARY})
ENDIF(HAVE_ZLIB)

# LZMA library. (7-Zip, .lzma, .xz)
IF(HAVE_LZMA)
	TARGET_LINK_LIBRARIES(gensfile ${LZMA_LIBRARY})
ENDIF(HAVE_LZMA)

# UnRAR. (TODO: HAVE_RAR?)
# We're not linking directly to UnRAR, so we need
# libdl on systems that use dlopen().
IF(CMAKE_DL_LIBS)
	TARGET_LINK_LIBRARIES(gensfile ${CMAKE_DL_LIBS})
ENDIF(CMAKE_DL_LIBS)

IF(WIN32)
	# version.dll is required for DcRar_Win32.cpp, which calls these functions:
	# - GetFileVersionInfoSize()
	# - GetFileVersionInfo()
	# - VerQueryValue()
	TARGET_LINK_LIBRARIES(gensfile version)
	TARGET_LINK_LIBRARIES(gensfile compat_W32U)
	IF(HAVE_MINIZIP)
		# MiniZip Win32 UTF-8 compatibility library.
		TARGET_LINK_LIBRARIES(gensfile compat_W32U_minizip)
	ENDIF(HAVE_MINIZIP)
ENDIF(WIN32)

# Set the compile definitions.
ADD_DEFINITIONS(${ZLIB_DEFINITIONS} ${MINIZIP_DEFINITIONS})
