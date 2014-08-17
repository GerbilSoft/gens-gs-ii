PROJECT(tools)
cmake_minimum_required(VERSION 2.6)

SET(MCD_PCM_PDB_FILE "${CMAKE_CURRENT_BINARY_DIR}/mcd_pcm.pdb")

IF(MSVC)
	# CMake seems to use weird settings for the PDB file.
	# (at least version 2.8.12.2; 3.0.0 might be different)
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_C}"
		"/Fd${MCD_PCM_PDB_FILE}"
		CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT}")
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_CXX}"
		"/Fd${MCD_PCM_PDB_FILE}"
		CMAKE_CXX_COMPILE_OBJECT "${CMAKE_CXX_COMPILE_OBJECT}")
ENDIF(MSVC)

# TODO: Header checks.
ADD_EXECUTABLE(mcd_pcm mcd_pcm.c)
