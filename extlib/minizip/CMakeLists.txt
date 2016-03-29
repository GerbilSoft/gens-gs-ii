PROJECT(minizip)
cmake_minimum_required(VERSION 2.6.0)

# Sources.
SET(minizip_SRCS
	unzip.c
	zip.c
	ioapi.c
	)
SET(minizip_H
	crypt.h
	ioapi.h
	iowin32.h
	unzip.h
	zip.h

	# Custom headers.
	mz_of.h
	)

IF(WIN32)
	# Windows-specific I/O handler.
	SET(minizip_SRCS ${minizip_SRCS} iowin32.c)
ENDIF(WIN32)

# ZLIB include directory.
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

######################
# Build the library. #
######################

ADD_LIBRARY(minizip STATIC ${minizip_SRCS})
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(minizip)
TARGET_LINK_LIBRARIES(minizip ${ZLIB_LIBRARY})
ADD_DEFINITIONS(${ZLIB_DEFINITIONS})

# Exclude from ALL and DEFAULT builds.
SET_TARGET_PROPERTIES(minizip PROPERTIES
	EXCLUDE_FROM_ALL TRUE
	EXCLUDE_FROM_DEFAULT_BUILD TRUE
	)
