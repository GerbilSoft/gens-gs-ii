PROJECT(libgens)
cmake_minimum_required(VERSION 2.6)

# LibGens subprojects.
SUBDIRS(starscream mdZ80)

# SDL is required for LibGens.
FIND_PACKAGE(SDL REQUIRED)
INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR})

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Sources.
SET(libgens_SRCS
	lg_main.cpp
	MtQueue.cpp
	SdlVideo.cpp
	MD/VdpIo.cpp
	MD/VdpRend.cpp
	MD/VdpRend_m5.cpp
	MD/TAB336.c
	)

# Compatibility sources.
IF(WIN32)
	SET(libgens_COMPAT_SRCS lg_win32.c)
ELSE()
	SET(libgens_COMPAT_SRCS)
ENDIF()

# Build the library.
ADD_LIBRARY(gens STATIC ${libgens_SRCS} ${libgens_COMPAT_SRCS})
TARGET_LINK_LIBRARIES(gens ${SDL_LIBRARY} starscream mdZ80)
ADD_DEPENDENCIES(gens git_version starscream mdZ80)
