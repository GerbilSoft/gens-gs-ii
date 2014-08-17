PROJECT(libgenskeys)
cmake_minimum_required(VERSION 2.6.0)

# Sources.
SET(libgenskeys_SRCS
	KeyManager.cpp
	)
SET(libgenskeys_H
	KeyManager.hpp
	)

######################
# Build the library. #
######################

SET(LIBGENSKEYS_PDB_FILE "${CMAKE_CURRENT_BINARY_DIR}/genskeys.pdb")

IF(MSVC)
	# CMake seems to use weird settings for the PDB file.
	# (at least version 2.8.12.2; 3.0.0 might be different)
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_C}"
		"/Fd${LIBGENSKEYS_PDB_FILE}"
		CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT}")
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_CXX}"
		"/Fd${LIBGENSKEYS_PDB_FILE}"
		CMAKE_CXX_COMPILE_OBJECT "${CMAKE_CXX_COMPILE_OBJECT}")
ENDIF(MSVC)

ADD_LIBRARY(genskeys STATIC
	${libgenskeys_SRCS}
	${libgenskeys_H}
	)
TARGET_LINK_LIBRARIES(genskeys gens)
