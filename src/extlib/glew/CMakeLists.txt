PROJECT(GLEW)
cmake_minimum_required(VERSION 2.6.0)

# TODO: Search for OpenGL here?

# Sources.
SET(GLEW_SRCS
	glew.c
	)

# Include the GLEW include/ directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/include/")

# Build the library.
ADD_LIBRARY(GLEW STATIC ${GLEW_SRCS})
TARGET_LINK_LIBRARIES(GLEW ${OPENGL_gl_LIBRARY})

# GLEW_STATIC needs to be defined for the static version.
SET_TARGET_PROPERTIES(GLEW
	PROPERTIES COMPILE_DEFINITIONS "GLEW_STATIC")
