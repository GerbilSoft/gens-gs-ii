# Source: http://www.cmake.org/Wiki/CMake/Assembler#NASM

SET(ASM_DIALECT "-NASM")
SET(CMAKE_ASM${ASM_DIALECT}_SOURCE_FILE_EXTENSIONS nasm;nas;asm)

# -f elf needed for linux. nasm -hf lists many formats, and default is 'bin'
# which doesnt work. See also the post by P. Punnoor to the CMake mailing-
# -list at http://www.cmake.org/pipermail/cmake/2005-November/007478.html
# TODO: Disable debugging symbols if debugging isn't requested.
# TODO: 64-bit.
# TODO: Other systems.
IF(UNIX)
	SET(CMAKE_ASM-NASM_COMPILER_ARG1 "-f elf -g -F dwarf")
ENDIF(UNIX)
IF(WIN32)
	SET(CMAKE_ASM-NASM_COMPILER_ARG1 "-f win32 -g -F stabs")
ENDIF(WIN32)
IF(APPLE)
	SET(CMAKE_ASM-NASM_COMPILER_ARG1 "-f macho -g -F null")
ENDIF(APPLE)

# Check for a release build.
IF(CMAKE_BUILD_TYPE MATCHES ^release$)
	SET(CMAKE_ASM-NASM_COMPILER_ARG1 "${CMAKE_ASM-NASM_COMPILER_ARG1} -O3")
ENDIF(CMAKE_BUILD_TYPE MATCHES ^release$)

# This section exists to override the one in CMakeASMInformation.cmake 
# (the default Information file). This removes the <FLAGS>
# thing so that your C compiler flags that have been set via 
# set_target_properties don't get passed to nasm and confuse it. 
IF(NOT CMAKE_ASM${ASM_DIALECT}_COMPILE_OBJECT)
	SET(CMAKE_ASM${ASM_DIALECT}_COMPILE_OBJECT "<CMAKE_ASM${ASM_DIALECT}_COMPILER> -o <OBJECT> <SOURCE>")
ENDIF(NOT CMAKE_ASM${ASM_DIALECT}_COMPILE_OBJECT)

INCLUDE(CMakeASMInformation)
SET(ASM_DIALECT)
