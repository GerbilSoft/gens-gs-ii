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

# Build the library.
ENABLE_LANGUAGE(ASM_NASM)
ADD_LIBRARY(mdZ80 STATIC ${SRCS})
