# Source: http://public.kitware.com/Bug/bug_view_page.php?bug_id=10069

# Find the nasm assembler

SET(ASM_DIALECT "_NASM")

SET(CMAKE_ASM${ASM_DIALECT}_COMPILER_INIT nasm)

IF(NOT CMAKE_ASM${ASM_DIALECT}_COMPILER)
	FIND_PROGRAM(CMAKE_ASM${ASM_DIALECT}_COMPILER nasm
		"$ENV{ProgramFiles}/NASM")
ENDIF(NOT CMAKE_ASM${ASM_DIALECT}_COMPILER)

INCLUDE(CMakeDetermineASMCompiler)
SET(ASM_DIALECT)
