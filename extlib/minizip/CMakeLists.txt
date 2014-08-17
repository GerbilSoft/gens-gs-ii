PROJECT(minizip)
cmake_minimum_required(VERSION 2.6.0)

# Sources.
SET(minizip_SRCS
	unzip.c
	zip.c
	ioapi.c
	)

# Platform-specific sources.
IF(WIN32)
	SET(minizip_PLATFORM_SRCS iowin32.c iowin32u.c)
ENDIF(WIN32)

# ZLIB include directory.
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

######################
# Build the library. #
######################

SET(MINIZIP_PDB_FILE "${CMAKE_CURRENT_BINARY_DIR}/minizip.pdb")

IF(MSVC)
	# CMake seems to use weird settings for the PDB file.
	# (at least version 2.8.12.2; 3.0.0 might be different)
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_C}"
		"/Fd${MINIZIP_PDB_FILE}"
		CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT}")
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_CXX}"
		"/Fd${MINIZIP_PDB_FILE}"
		CMAKE_CXX_COMPILE_OBJECT "${CMAKE_CXX_COMPILE_OBJECT}")
ENDIF(MSVC)

ADD_LIBRARY(minizip STATIC ${minizip_SRCS} ${minizip_PLATFORM_SRCS})
TARGET_LINK_LIBRARIES(minizip ${ZLIB_LIBRARY})
