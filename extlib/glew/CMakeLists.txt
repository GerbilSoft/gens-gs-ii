PROJECT(glew)
cmake_minimum_required(VERSION 2.6.0)

# NOTE: Project name is lowercase "glew" to prevent
# naming conflicts with system libGLEW.so.

# TODO: Search for OpenGL here?

# Sources.
SET(glew_SRCS
	glew.c
	)

# Include the GLEW include/ directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/include/")

######################
# Build the library. #
######################

ADD_LIBRARY(glew STATIC ${glew_SRCS})
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(glew)
TARGET_LINK_LIBRARIES(glew ${OPENGL_gl_LIBRARY})

# GLEW_STATIC needs to be defined for the static version.
SET_TARGET_PROPERTIES(glew
	PROPERTIES COMPILE_DEFINITIONS "GLEW_STATIC")

# Exclude from ALL and DEFAULT builds.
SET_TARGET_PROPERTIES(glew PROPERTIES
	EXCLUDE_FROM_ALL TRUE
	EXCLUDE_FROM_DEFAULT_BUILD TRUE
	)
