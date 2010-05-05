PROJECT(minizip)
cmake_minimum_required(VERSION 2.6.0)

# zlib is required for MiniZip.
FIND_PACKAGE(ZLIB REQUIRED)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

# Sources.
SET(minizip_SRCS
	unzip.c
	zip.c
	)

# Platform-specific sources.
IF(WIN32)
	SET(minizip_PLATFORM_SRCS io_win32.c)
ELSE()
	SET(minizip_PLATFORM_SRCS ioapi.c)
ENDIF()

# Build the library.
ADD_LIBRARY(minizip STATIC ${minizip_SRCS} ${minizip_PLATFORM_SRCS})
TARGET_LINK_LIBRARIES(minizip ${ZLIB_LIBRARY})
