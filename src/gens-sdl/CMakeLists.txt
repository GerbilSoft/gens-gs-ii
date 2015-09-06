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
INCLUDE_DIRECTORIES(${GLEW_INCLUDE_DIR})

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.gens-sdl.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.gens-sdl.h")

# Sources.
SET(gens-sdl_SRCS
	gens-sdl.cpp
	EventLoop.cpp
	EmuLoop.cpp
	CrazyEffectLoop.cpp
	SdlHandler.cpp
	SdlHandler_scancode.cpp
	RingBuffer.cpp
	Config.cpp
	VBackend.cpp
	SdlSWBackend.cpp
	SdlGLBackend.cpp
	GLBackend.cpp
	str_lookup.cpp
	GLTex.cpp
	)

# Headers.
SET(gens-sdl_H
	gens-sdl.hpp
	EventLoop.hpp
	EventLoop_p.hpp
	EmuLoop.hpp
	CrazyEffectLoop.hpp
	SdlHandler.hpp
	RingBuffer.hpp
	Config.hpp
	VBackend.hpp
	SdlSWBackend.hpp
	SdlGLBackend.hpp
	GLBackend.hpp
	str_lookup.hpp
	GLTex.hpp
	)

# OSD sources.
SET(gens-sdl_OSD_SRCS
	OsdGL.cpp
	OsdFont_VGA.cpp
	OsdFont_C64.cpp
	)

# OSD headers.
SET(gens-sdl_OSD_H
	OsdGL.hpp
	OsdFont_VGA.hpp
	OsdFont_C64.hpp
	)

# Shader sources.
SET(gens-sdl_SHADER_SRCS
	GLShader.cpp
	GLShaderFastBlur.cpp
	)

# Shader headers.
SET(gens-sdl_SHADER_H
	GLShader.hpp
	GLShaderFastBlur.hpp
	)

# Win32 resources.
IF(WIN32)
	SET(gens-sdl_PLATFORM_SRCS
		win32/gens-sdl.rc
		)
ENDIF(WIN32)

# Main target.
ADD_EXECUTABLE(gens-sdl
	${gens-sdl_SRCS}
	${gens-sdl_H}
	${gens-sdl_OSD_SRCS}
	${gens-sdl_OSD_H}
	${gens-sdl_SHADER_SRCS}
	${gens-sdl_SHADER_H}
	${gens-sdl_PLATFORM_SRCS}
	)
TARGET_LINK_LIBRARIES(gens-sdl compat gens genskeys zomg)
DO_SPLIT_DEBUG(gens-sdl)

# Additional libraries.
TARGET_LINK_LIBRARIES(gens-sdl
	${SDL2_LIBRARY}
	${OPENGL_gl_LIBRARY}
	${GLEW_LIBRARY}
	)

# Set the compile definitions.
ADD_DEFINITIONS(${GLEW_DEFINITIONS})

# Compress the EXE.
IF(COMPRESS_EXE)
	INCLUDE(CompressExeWithUpx)
	COMPRESS_EXE_WITH_UPX(gens-sdl)
ENDIF(COMPRESS_EXE)
