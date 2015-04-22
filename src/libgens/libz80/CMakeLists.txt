PROJECT(libz80)

# FILE command requires CMake 2.8.0.
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)

# Create the codegen directory.
# NOTE: This only runs at configure time...
FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/codegen)
FILE(COPY
	${CMAKE_CURRENT_SOURCE_DIR}/codegen/mktables.spec
	${CMAKE_CURRENT_SOURCE_DIR}/codegen/opcodes.lst
	DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/codegen
	)

# Compile the opcode table generator.
ADD_EXECUTABLE(mktables codegen/mktables.c)

# Generate the opcode table.
# FIXME: Get a copy of GNU regex from libc (posix/regex.c and posix/regex.h)
# and use it if the system doesn't have regex.h.
ADD_CUSTOM_COMMAND(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/codegen/opcodes_impl.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/codegen
	DEPENDS
		mktables
		${CMAKE_CURRENT_BINARY_DIR}/codegen/mktables.spec
		${CMAKE_CURRENT_BINARY_DIR}/codegen/opcodes.lst
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/mktables${CMAKE_EXECUTABLE_SUFFIX}
	)

# Generate the header for the opcode table.
# TODO: Requires grep and sed.
# TODO: Maybe make mktables generate the header file?
# FIXME: ADD_CUSTOM_COMMAND won't delete the redirected file on error...
ADD_CUSTOM_COMMAND(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/codegen/opcodes_decl.h
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/codegen/opcodes_impl.c
	COMMAND grep -F "static void" "${CMAKE_CURRENT_BINARY_DIR}/codegen/opcodes_impl.c" | sed -e \"s/\\x24/\\x3B/g\" >"${CMAKE_CURRENT_BINARY_DIR}/codegen/opcodes_decl.h"
	)

# libz80 library.
# FIXME: Set the PDB filename.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_LIBRARY(z80 STATIC z80.c)
SET_SOURCE_FILES_PROPERTIES(z80.c
	PROPERTIES OBJECT_DEPENDS
		${CMAKE_CURRENT_BINARY_DIR}/codegen/opcodes_decl.h
	)

#ENABLE_LANGUAGE(ASM_NASM)
#ADD_LIBRARY(starscream STATIC
#	${CMAKE_CURRENT_BINARY_DIR}/main68k.asm
#	${CMAKE_CURRENT_BINARY_DIR}/sub68k.asm
#	asm-wrapper.asm
#
#	# Dummy C file to force CMake to handle this as a C library.
#	# (CMake's NMake generator can't handle a pure ASM_NASM library.)
#	dummy.c
#	)
