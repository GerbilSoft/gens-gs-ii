PROJECT(starscream)

# NASM requires CMake 2.8.4.
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)

# Compile the Starscream ASM generators.
IF(CMAKE_CROSSCOMPILING)
	# Cross-compiling.
	# Use executables from a native build.
	INCLUDE("${IMPORT_EXECUTABLES}/src/libgens/starscream/ImportExecutables.cmake")
ELSE(CMAKE_CROSSCOMPILING)
	# Not cross-compiling.
	ADD_EXECUTABLE(star_m68k star_m68k.c)
	DO_SPLIT_DEBUG(star_m68k)
	ADD_EXECUTABLE(star_s68k star_s68k.c)
	DO_SPLIT_DEBUG(star_s68k)

	# Export these executables for cross-compiling.
	# Reference: http://www.cmake.org/Wiki/CMake_Cross_Compiling
	EXPORT(TARGETS star_m68k star_s68k
		FILE "${CMAKE_CURRENT_BINARY_DIR}/ImportExecutables.cmake"
		)
ENDIF(CMAKE_CROSSCOMPILING)

# Special build commands for main68k.asm and sub68k.asm
ADD_CUSTOM_COMMAND(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/main68k.asm
	COMMAND star_m68k main68k.asm -hog -name main68k_
	DEPENDS star_m68k
	)
ADD_CUSTOM_COMMAND(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sub68k.asm
	COMMAND star_s68k sub68k.asm -hog -name sub68k_
	DEPENDS star_s68k
	)

# i386 assembler sources.
SET(starscream_ASM_NASM_SRCS
	${CMAKE_CURRENT_BINARY_DIR}/main68k.asm
	${CMAKE_CURRENT_BINARY_DIR}/sub68k.asm
	asm-wrapper.asm
	)

# Explicitly specify ASM_NASM as the source language.
SET_SOURCE_FILES_PROPERTIES(${starscream_ASM_NASM_SRCS}
	PROPERTIES LANGUAGE ASM_NASM)

# Starscream library.
ENABLE_LANGUAGE(ASM_NASM)
ADD_LIBRARY(starscream STATIC
	${starscream_ASM_NASM_SRCS}

	# Dummy C file to force CMake to handle this as a C library.
	# (CMake's NMake generator can't handle a pure ASM_NASM library.)
	dummy.c
	)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(starscream)
