PROJECT(gens-sdl)
cmake_minimum_required(VERSION 2.6)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES("${gens-gs-ii_BINARY_DIR}")

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../")

# gens-sdl source and binary directories.
# Binary directory is needed for config.h.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# Find SDL.
SET(SDL2_BUILDING_LIBRARY ON)
FIND_PACKAGE(SDL2 REQUIRED)
INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIR})

# ZLIB include directory.
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

# Include the OpenGL directory.
# TODO: FIND_PACKAGE(OpenGL) here even though it's in the root CMakeLists.txt?
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})

# Sources.
SET(gens-sdl_SRCS
	gens-sdl.cpp
	SdlHandler.cpp
	SdlHandler_scancode.cpp
	RingBuffer.cpp
	Config.cpp
	VBackend.cpp
	SdlSWBackend.cpp
	SdlGLBackend.cpp
	GLBackend.cpp
	str_lookup.cpp
	OsdGL.cpp
	OsdFont_VGA.cpp
	GLTex.cpp
	)

# Headers.
SET(gens-sdl_H
	gens-sdl.cpp
	SdlHandler.hpp
	RingBuffer.hpp
	Config.hpp
	VBackend.hpp
	SdlSWBackend.hpp
	SdlGLBackend.hpp
	GLBackend.hpp
	str_lookup.hpp
	OsdGL.hpp
	OsdFont_VGA.hpp
	GLTex.hpp
	)

# Main target.
ADD_EXECUTABLE(gens-sdl
	${gens-sdl_SRCS}
	${gens-sdl_H}
	)
TARGET_LINK_LIBRARIES(gens-sdl compat gens genskeys zomg)

# Additional libraries.
TARGET_LINK_LIBRARIES(gens-sdl
	${SDL2_LIBRARY}
	${OPENGL_gl_LIBRARY}
	)

# Split debug information.
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(gens-sdl)
IF(SPLIT_DEBUG)
	INCLUDE(SplitDebugInformation)
	SPLIT_DEBUG_INFORMATION(gens-sdl)
ENDIF(SPLIT_DEBUG)

# Compress the EXE.
IF(COMPRESS_EXE)
	INCLUDE(CompressExeWithUpx)
	COMPRESS_EXE_WITH_UPX(gens-sdl)
ENDIF(COMPRESS_EXE)
