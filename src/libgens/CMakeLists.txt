PROJECT(libgens)
cmake_minimum_required(VERSION 2.6.0)

# LibGens subprojects.
ADD_SUBDIRECTORY(starscream)
ADD_SUBDIRECTORY(mdZ80)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# LibGens source and binary directories.
# Binary directory is needed for config.h.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# Library checks.
INCLUDE(CheckLibraryExists)
CHECK_LIBRARY_EXISTS(c sigaction "" HAVE_SIGACTION)	# libc: sigaction()

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h")

# Sources.
SET(libgens_SRCS
	lg_main.cpp
	MD/VdpIo.cpp
	MD/VdpRend.cpp
	MD/VdpRend_m5.cpp
	MD/TAB336.c
	MD/EmuMD.cpp
	MD/VdpPalette.cpp
	Save/Zomg.cpp
	cpu/M68K.cpp
	cpu/M68K_Mem.cpp
	sound/Psg.cpp
	sound/Ym2612.cpp
	macros/log_msg.c
	Util/Effects.cpp
	Util/byteswap.c
	Rom.cpp
	Util/siginfo.c
	Util/cpuflags.c
	)

# Compatibility sources.
IF(WIN32)
	SET(libgens_COMPAT_SRCS lg_win32.c)
ELSE()
	SET(libgens_COMPAT_SRCS)
ENDIF()

# Build the library.
ADD_LIBRARY(gens STATIC ${libgens_SRCS} ${libgens_COMPAT_SRCS})
TARGET_LINK_LIBRARIES(gens starscream mdZ80 minizip)
ADD_DEPENDENCIES(gens git_version starscream mdZ80 minizip)
