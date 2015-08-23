# - Check if the system is x86 (32-bit).
# CHECK_SYSTEM_X86_32(VARIABLE)
#
#  VARIABLE - variable to store the result
# 
#  This actually calls the check_c_source_compiles macro.
#  See help for CheckCSourceCompiles for a listing of variables
#  that can modify the build.

# Copyright (c) 2010, David Korth, <gerbilsoft@verizon.net>

# Based on CHECK_C99_COMPILER_FLAG().
# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Based on CHECK_C_COMPILER_FLAG(). (CheckCCompilerFlag.cmake)

INCLUDE(CheckCSourceCompiles)

MACRO(CHECK_SYSTEM_X86_32 _RESULT)
	# Flag listing borrowed from GNU autoconf's AC_PROG_CC_C99 macro.
	IF(NOT DEFINED CHECK_X86_32)
		UNSET(${_RESULT})

		MESSAGE(STATUS "Checking if the system is X86_32:")

		CHECK_C_SOURCE_COMPILES(
			"#if (defined(__i386) || defined(__i386__) || defined(_M_IX86)) && (!defined(__x86_64) && !defined(__x86_64__) && !defined(__amd64) && !defined(__amd64__) && !defined(_M_X64) && !defined(_M_AMD64))
			/* System is X86_32 */
			#else
			/* System is not X86_32 */
			#error Not X86_32. Aborting.
			#endif

			int main(void) { return 0; }" CHECK_X86_32)

		IF(CHECK_X86_32)
			SET(CHECKED_X86_32 1 CACHE INTERNAL "Is the system X86_32?")
			MESSAGE(STATUS "Checking if the system is X86_32: yes")
		ELSE(CHECK_X86_32)
			SET(CHECKED_X86_32 0 CACHE INTERNAL "Is the system X86_32?")
			MESSAGE(STATUS "Checking if the system is X86_32: no")
		ENDIF(CHECK_X86_32)
	ENDIF(NOT DEFINED CHECK_X86_32)
	SET(${_RESULT} ${CHECK_X86_32})
ENDMACRO (CHECK_SYSTEM_X86_32)
