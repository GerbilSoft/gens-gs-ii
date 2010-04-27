# - Check what flag is needed to activate C99 mode.
# CHECK_C99_COMPILER_FLAG(VARIABLE)
#
#  VARIABLE - variable to store the result
# 
#  This actually calls the check_c_source_compiles macro.
#  See help for CheckCSourceCompiles for a listing of variables
#  that can modify the build.

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Based on CHECK_C_COMPILER_FLAG(). (CheckCCompilerFlag.cmake)

INCLUDE(CheckCSourceCompiles)

MACRO(CHECK_C99_COMPILER_FLAG _RESULT)
	# Flag listing borrowed from GNU autoconf's AC_PROG_CC_C99 macro.
	SET(${_RESULT} "")
	MESSAGE(STATUS "Checking what CFLAG is required for C99:")
	FOREACH(C99_CFLAG "-std=gnu99" "-std=c99" "-c99" "-AC99" "-xc99=all" "-qlanglvl=extc99")
		SET(SAFE_CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS}")
		SET(CMAKE_REQUIRED_DEFINITIONS "${C99_FLAG}")
		CHECK_C_SOURCE_COMPILES("int main() { return 0;}" C99_FLAG_TEST)
		SET(CMAKE_REQUIRED_DEFINITIONS "${SAFE_CMAKE_REQUIRED_DEFINITIONS}")
		IF(C99_FLAG_TEST)
			SET(${_RESULT} ${C99_CFLAG})
			BREAK()
		ENDIF(C99_FLAG_TEST)
	ENDFOREACH()
	
	IF(${${_RESULT}})
		MESSAGE(STATUS "Checking what CFLAG is required for C99: ${${_RESULT}}")
	ELSE()
		MESSAGE(STATUS "Checking what CFLAG is required for C99: ${${_RESULT}}")
	ENDIF()
ENDMACRO (CHECK_C99_COMPILER_FLAG)
