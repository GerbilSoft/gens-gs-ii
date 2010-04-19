# Source: http://www.cmake.org/Wiki/CMake/Assembler#NASM

# determine the compiler to use for NASM

SET(ASM_DIALECT "-NASM")
SET(CMAKE_ASM${ASM_DIALECT}_COMPILER_INIT ${_CMAKE_TOOLCHAIN_PREFIX}nasm)
INCLUDE(CMakeDetermineASMCompiler)
SET(ASM_DIALECT)
