PROJECT(gens-gs-ii)
cmake_minimum_required(VERSION 2.6)

list (APPEND CMAKE_MODULE_PATH "${gens-gs-ii_SOURCE_DIR}/CMake")

# Git version information.
ADD_CUSTOM_TARGET(git_version ALL
	${gens-gs-ii_SOURCE_DIR}/git_version.sh -s "${gens-gs-ii_SOURCE_DIR}" -o "${gens-gs-ii_BINARY_DIR}/git_version.h"
	VERBATIM
)

# Hack to remove -rdynamic from CFLAGS and CXXFLAGS
# See http://public.kitware.com/pipermail/cmake/2006-July/010404.html
IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
	SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
ENDIF()

# Don't embed rpaths in the executables.
SET(CMAKE_SKIP_RPATH ON)

# If no build type is set, default to "debug".
IF(CMAKE_BUILD_TYPE MATCHES ^$)
	SET(CMAKE_BUILD_TYPE "debug")
ENDIF()

# Don't bother with verbose makefiles for now.
# TODO: Make it an option?
#IF (CMAKE_BUILD_TYPE MATCHES ^debug$)
#	SET(CMAKE_VERBOSE_MAKEFILE ON)
#ENDIF()

# Check what flag is needed for C99 support.
INCLUDE(CheckC99CompilerFlag)
CHECK_C99_COMPILER_FLAG(C99_CFLAG)

# Set CFLAGS/CXXFLAGS based on build type.
SET(CMAKE_C_FLAGS_DEBUG		"-Wall -Wextra -O0 ${C99_CFLAG}")
SET(CMAKE_CXX_FLAGS_DEBUG	"-Wall -Wextra -O0")
SET(CMAKE_C_FLAGS_RELEASE	"-Wall -Wextra -O2 -march=i586 -mtune=i686 ${C99_CFLAG}")
SET(CMAKE_CXX_FLAGS_RELEASE	"-Wall -Wextra -O2 -march=i586 -mtune=i686")

# Win32 CFLAGS.
# TODO: Test for available CFLAGS/CXXFLAGS/LDFLAGS.
# TODO: Static libgcc/libstdc++.
IF(WIN32)
	SET(CMAKE_C_FLAGS_DEBUG		"${CMAKE_C_FLAGS_DEBUG} -gstabs -fshort-wchar")
	SET(CMAKE_CXX_FLAGS_DEBUG	"${CMAKE_CXX_FLAGS_DEBUG} -gstabs -fshort-wchar")
	SET(CMAKE_C_FLAGS_RELEASE	"${CMAKE_C_FLAGS_RELEASE} -gstabs -fshort-wchar")
	SET(CMAKE_CXX_FLAGS_RELEASE	"${CMAKE_CXX_FLAGS_RELEASE} -gstabs -fshort-wchar")
ELSEIF(UNIX)
	SET(CMAKE_C_FLAGS_DEBUG		"${CMAKE_C_FLAGS_DEBUG} -ggdb")
	SET(CMAKE_CXX_FLAGS_DEBUG	"${CMAKE_CXX_FLAGS_DEBUG} -ggdb")
	SET(CMAKE_C_FLAGS_RELEASE	"${CMAKE_C_FLAGS_RELEASE} -ggdb")
	SET(CMAKE_CXX_FLAGS_RELEASE	"${CMAKE_CXX_FLAGS_RELEASE} -ggdb")
ENDIF()

# Project subdirectories
SUBDIRS(src)
