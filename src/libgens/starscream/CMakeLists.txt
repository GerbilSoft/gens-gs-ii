PROJECT(starscream)

# NASM requires CMake 2.8.4.
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)

# TODO: Cross-compile support.

# Compile the Starscream ASM generators.
ADD_EXECUTABLE(star_m68k star_m68k.c)
ADD_EXECUTABLE(star_s68k star_s68k.c)

# Special build commands for main68k.asm and sub68k.asm
# TODO: Cross-compiling support.
ADD_CUSTOM_COMMAND(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/main68k.asm
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/star_m68k${CMAKE_EXECUTABLE_SUFFIX} main68k.asm -hog -name main68k_
	DEPENDS star_m68k
	)
ADD_CUSTOM_COMMAND(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sub68k.asm
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/star_s68k${CMAKE_EXECUTABLE_SUFFIX} sub68k.asm -hog -name sub68k_
	DEPENDS star_s68k
	)

# Starscream library.
ENABLE_LANGUAGE(ASM_NASM)
ADD_LIBRARY(starscream STATIC ${CMAKE_CURRENT_BINARY_DIR}/main68k.asm ${CMAKE_CURRENT_BINARY_DIR}/sub68k.asm asm-wrapper.asm)
