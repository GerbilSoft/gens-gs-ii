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

SET(GLEW_PDB_FILE "${CMAKE_CURRENT_BINARY_DIR}/glew.pdb")

IF(MSVC)
	# CMake seems to use weird settings for the PDB file.
	# (at least version 2.8.12.2; 3.0.0 might be different)
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_C}"
		"/Fd${GLEW_PDB_FILE}"
		CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT}")
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_CXX}"
		"/Fd${GLEW_PDB_FILE}"
		CMAKE_CXX_COMPILE_OBJECT "${CMAKE_CXX_COMPILE_OBJECT}")
ENDIF(MSVC)

ADD_LIBRARY(glew STATIC ${glew_SRCS})
TARGET_LINK_LIBRARIES(glew ${OPENGL_gl_LIBRARY})

# GLEW_STATIC needs to be defined for the static version.
SET_TARGET_PROPERTIES(glew
	PROPERTIES COMPILE_DEFINITIONS "GLEW_STATIC")
