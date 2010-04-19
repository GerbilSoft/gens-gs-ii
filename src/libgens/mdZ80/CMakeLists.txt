PROJECT(mdZ80)
cmake_minimum_required(VERSION 2.6)

# Sources.
SET(SRCS
	mdZ80_x86.asm
	mdZ80.c
	mdZ80_DAA.c
	mdZ80_INC_DEC.c
	)

# Build the library.
ENABLE_LANGUAGE(ASM-NASM)
ADD_LIBRARY(mdZ80 STATIC ${SRCS})
