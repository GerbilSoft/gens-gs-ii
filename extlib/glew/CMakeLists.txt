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

# Build the library.
ADD_LIBRARY(glew STATIC ${glew_SRCS})
TARGET_LINK_LIBRARIES(glew ${OPENGL_gl_LIBRARY})

# GLEW_STATIC needs to be defined for the static version.
SET_TARGET_PROPERTIES(glew
	PROPERTIES COMPILE_DEFINITIONS "GLEW_STATIC")
