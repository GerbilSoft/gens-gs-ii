PROJECT(libzomg)
cmake_minimum_required(VERSION 2.6.0)

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../")

# LibGens source and binary directories.
# Binary directory is needed for config.h.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# ZLIB and MINIZIP include directories.
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${MINIZIP_INCLUDE_DIR})

# libpng
INCLUDE(CheckPNG)
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.libzomg.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.libzomg.h")

# Sources.
SET(libzomg_SRCS
	ZomgBase.cpp
	Zomg.cpp
	ZomgLoad.cpp
	ZomgSave.cpp
	Metadata.cpp
	PngWriter.cpp
	PngReader.cpp
	)
IF(WIN32)
	SET(libzomg_SRCS
		${libzomg_SRCS}
		Metadata_win32.cpp
		)
ELSE()
	SET(libzomg_SRCS
		${libzomg_SRCS}
		Metadata_unix.cpp
		)
ENDIF()

# Headers.
SET(libzomg_H
	ZomgBase.hpp
	Zomg.hpp
	Zomg_p.hpp
	Metadata.hpp
	PngWriter.hpp
	PngReader.hpp
	)

# ZOMG struct headers.
SET(libzomg_struct_H
	zomg_byteorder.h
	zomg_common.h
	zomg_vdp.h
	zomg_psg.h
	zomg_ym2612.h
	zomg_m68k.h
	zomg_z80.h
	zomg_md_io.h
	zomg_md_z80_ctrl.h
	zomg_md_time_reg.h
	zomg_md_tmss_reg.h
	)

######################
# Build the library. #
######################

ADD_LIBRARY(zomg STATIC
	${libzomg_SRCS}
	${libzomg_H}
	${libzomg_struct_H}
	)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(zomg)
TARGET_LINK_LIBRARIES(zomg compat ${MINIZIP_LIBRARY} ${PNG_LIBRARY})
IF(WIN32)
	# Secur32.dll is required for Metadata_win32.cpp, which calls these functions:
	# - GetUserNameEx()
	# Advapi32.dll is required for Metadata_win32.cpp, which calls these functions:
	# - RegOpenKeyEx()
	# - RegQueryValueEx()
	# - RegCloseKey()
	TARGET_LINK_LIBRARIES(zomg secur32 advapi32)
ENDIF(WIN32)

# Set the compile definitions.
ADD_DEFINITIONS(${ZLIB_DEFINITIONS} ${MINIZIP_DEFINITIONS} ${PNG_DEFINITIONS})

# Test suite.
IF(BUILD_TESTING)
       ADD_SUBDIRECTORY(tests)
ENDIF(BUILD_TESTING)
