PROJECT(mdZ80)

# NASM requires CMake 2.8.4.
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)

# Sources.
SET(mdZ80_SRCS
	mdZ80.c
	mdZ80_reg.c
	mdZ80_DAA.c
	mdZ80_INC_DEC.c
	)

# i386 assembler sources.
SET(mdZ80_ASM_NASM_SRCS
	mdZ80_x86.asm
	)

# Explicitly specify ASM_NASM as the source language.
SET_SOURCE_FILES_PROPERTIES(${mdZ80_ASM_NASM_SRCS}
	PROPERTIES LANGUAGE ASM_NASM)

######################
# Build the library. #
######################

ENABLE_LANGUAGE(ASM_NASM)
ADD_LIBRARY(mdZ80 STATIC
	${mdZ80_SRCS}
	${mdZ80_ASM_NASM_SRCS}
	)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(mdZ80)
