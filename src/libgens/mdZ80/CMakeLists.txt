PROJECT(mdZ80)

# NASM requires CMake 2.8.4.
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)

# Sources.
SET(SRCS
	mdZ80_x86.asm
	mdZ80.c
	mdZ80_reg.c
	mdZ80_DAA.c
	mdZ80_INC_DEC.c
	)

######################
# Build the library. #
######################

SET(MDZ80_PDB_FILE "${CMAKE_CURRENT_BINARY_DIR}/mdZ80.pdb")

IF(MSVC)
	# CMake seems to use weird settings for the PDB file.
	# (at least version 2.8.12.2; 3.0.0 might be different)
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_C}"
		"/Fd${MDZ80_PDB_FILE}"
		CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT}")
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_CXX}"
		"/Fd${MDZ80_PDB_FILE}"
		CMAKE_CXX_COMPILE_OBJECT "${CMAKE_CXX_COMPILE_OBJECT}")
ENDIF(MSVC)

ENABLE_LANGUAGE(ASM_NASM)
ADD_LIBRARY(mdZ80 STATIC ${SRCS})
