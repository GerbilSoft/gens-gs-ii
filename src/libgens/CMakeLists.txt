PROJECT(libgens)
cmake_minimum_required(VERSION 2.6.0)

# LibGens subprojects.
ADD_SUBDIRECTORY(starscream)
ADD_SUBDIRECTORY(mdZ80)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# LibGens source directory.
INCLUDE_DIRECTORIES(${libgens_SOURCE_DIR})

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
	Effects.cpp
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
