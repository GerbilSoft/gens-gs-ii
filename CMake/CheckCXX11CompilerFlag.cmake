# - Check what flag is needed to activate C++ 2011 mode.
# CHECK_CXX11_COMPILER_FLAG(VARIABLE)
#
#  VARIABLE - variable to store the result
# 
#  This actually calls the check_c_source_compiles macro.
#  See help for CheckCSourceCompiles for a listing of variables
#  that can modify the build.

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
# C++ 2011 version Copyright (c) 2011 by David Korth.
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Based on CHECK_C99_COMPILER_FLAG(). (CheckC99CompilerFlag.cmake)

INCLUDE(CheckCXXSourceCompiles)

MACRO(CHECK_CXX11_COMPILER_FLAG _RESULT)
	UNSET(${_RESULT})
	
	MESSAGE(STATUS "Checking what CXXFLAG is required for C++ 2011:")
	FOREACH(CHECK_CXX11_CXXFLAG "-std=gnu++0x" "-std=c++0x")
		SET(SAFE_CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS}")
		SET(CMAKE_REQUIRED_DEFINITIONS "${CHECK_CXX11_CXXFLAG}")
		CHECK_CXX_SOURCE_COMPILES("int main() { void *ptr = nullptr; return 0; }" CXXFLAG_TO_CHECK)
		SET(CMAKE_REQUIRED_DEFINITIONS "${SAFE_CMAKE_REQUIRED_DEFINITIONS}")
		IF(CXXFLAG_TO_CHECK)
			SET(${_RESULT} ${CHECK_CXX11_CXXFLAG})
			BREAK()
		ENDIF(CXXFLAG_TO_CHECK)
		UNSET(CXXFLAG_TO_CHECK)
	ENDFOREACH()
	
	IF(${_RESULT})
		MESSAGE(STATUS "Checking what CXXFLAG is required for C++ 2011: ${${_RESULT}}")
	ELSE(${_RESULT})
		MESSAGE(STATUS "Checking what CXXFLAG is required for C++ 2011: none")
	ENDIF(${_RESULT})
ENDMACRO(CHECK_CXX11_COMPILER_FLAG)
