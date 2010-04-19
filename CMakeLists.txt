PROJECT(gens-gs-ii)
cmake_minimum_required(VERSION 2.6)

list (APPEND CMAKE_MODULE_PATH "${gens-gs-ii_SOURCE_DIR}/CMake")

# Git version information.
ADD_CUSTOM_TARGET(git_version ALL
	${gens-gs-ii_SOURCE_DIR}/git_version.sh -o "${gens-gs-ii_BINARY_DIR}/git_version.h"
	VERBATIM
)

#if you don't want the full compiler output, remove the following line
#SET(CMAKE_VERBOSE_MAKEFILE ON)

# Hack to remove -rdynamic from CFLAGS and CXXFLAGS
# See http://public.kitware.com/pipermail/cmake/2006-July/010404.html
IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
	SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
ENDIF(CMAKE_SYSTEM_NAME STREQUAL "Linux")

# Don't embed rpaths in the executables.
SET(CMAKE_SKIP_RPATH ON)

#add definitions, compiler switches, etc.
ADD_DEFINITIONS(-Wall -Wextra -O0 -ggdb)

# Project subdirectories
SUBDIRS(src)
