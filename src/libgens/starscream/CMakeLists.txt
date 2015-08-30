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
	ADD_EXECUTABLE(star_s68k star_s68k.c)

	# Export these executables for cross-compiling.
	# Reference: http://www.cmake.org/Wiki/CMake_Cross_Compiling
	EXPORT(TARGETS star_m68k star_s68k
		FILE "${CMAKE_CURRENT_BINARY_DIR}/ImportExecutables.cmake"
		)
ENDIF(CMAKE_CROSSCOMPILING)

# NOTE: $<TARGET_FILE:gcbanner> is preferred,
# but this doesn't seem to work on Ubuntu 10.04.
# (cmake_2.8.0-5ubuntu1_i386)
GET_PROPERTY(STAR_M68K_EXE TARGET star_m68k PROPERTY LOCATION)
GET_PROPERTY(STAR_S68K_EXE TARGET star_s68k PROPERTY LOCATION)

# Special build commands for main68k.asm and sub68k.asm
ADD_CUSTOM_COMMAND(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/main68k.asm
	COMMAND ${STAR_M68K_EXE} main68k.asm -hog -name main68k_
	DEPENDS star_m68k
	)
ADD_CUSTOM_COMMAND(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sub68k.asm
	COMMAND ${STAR_S68K_EXE} sub68k.asm -hog -name sub68k_
	DEPENDS star_s68k
	)

# Starscream library.
# FIXME: Set the PDB filename.
ENABLE_LANGUAGE(ASM_NASM)
ADD_LIBRARY(starscream STATIC
	${CMAKE_CURRENT_BINARY_DIR}/main68k.asm
	${CMAKE_CURRENT_BINARY_DIR}/sub68k.asm
	asm-wrapper.asm

	# Dummy C file to force CMake to handle this as a C library.
	# (CMake's NMake generator can't handle a pure ASM_NASM library.)
	dummy.c
	)
