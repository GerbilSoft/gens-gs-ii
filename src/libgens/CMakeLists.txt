PROJECT(libgens)
cmake_minimum_required(VERSION 2.6)

# LibGens subprojects.
SUBDIRS(starscream mdZ80)

# SDL is required for LibGens.
FIND_PACKAGE(SDL REQUIRED)
INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR})

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Compatibility sources.
IF(WIN32)
	SET(libgens_COMPAT_SRC lg_win32.c)
ELSE()
	SET(libgens_COMPAT_SRC)
ENDIF()

# Sources.
SET(SRCS
	lg_main.cpp
	MtQueue.cpp
	SdlVideo.cpp
	${libgens_COMPAT_SRC}
	)

# Build the library.
ADD_LIBRARY(gens STATIC ${SRCS})
TARGET_LINK_LIBRARIES(gens ${SDL_LIBRARY} starscream mdZ80)
ADD_DEPENDENCIES(gens git_version starscream mdZ80)
