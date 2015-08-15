PROJECT(src)
cmake_minimum_required(VERSION 2.6.0)

# Include the current source and binary directories.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

# Include the extlib directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../extlib/")

# C++11 compatibility header.
# NOTE: This must be included regardless of C++11 support in the compiler.
# gcc-4.6 supports some C++11, but is missing explicit virtual overrides.
IF(MSVC)
	SET(GENS_CXX11_COMPAT_HEADER "-FI${CMAKE_CURRENT_SOURCE_DIR}/c++11-compat.h")
ELSE(MSVC)
	SET(GENS_CXX11_COMPAT_HEADER "-include ${CMAKE_CURRENT_SOURCE_DIR}/c++11-compat.h")
ENDIF(MSVC)
SET(GENS_CXXFLAG_CXX11_COMPAT ${GENS_CXX11_COMPAT_HEADER})

# Add the C++11 compatibility header to CFLAGS/CXXFLAGS.
SET(CMAKE_C_FLAGS	"${CMAKE_C_FLAGS} ${GENS_CXX11_COMPAT_HEADER}")
SET(CMAKE_CXX_FLAGS	"${CMAKE_CXX_FLAGS} ${GENS_CXXFLAG_CXX11_COMPAT}")

# Compatibility library. (Includes W32U Mini.)
ADD_SUBDIRECTORY(libcompat)

# Source Code subdirectories.
ADD_SUBDIRECTORY(libzomg)
ADD_SUBDIRECTORY(libgenstext)
ADD_SUBDIRECTORY(libgens)
ADD_SUBDIRECTORY(libgenscd)
ADD_SUBDIRECTORY(libgenskeys)
#ADD_SUBDIRECTORY(tools)

IF(ENABLE_GENS_QT4)
	ADD_SUBDIRECTORY(gens-qt4)
ENDIF(ENABLE_GENS_QT4)
IF(ENABLE_GENS_SDL)
	ADD_SUBDIRECTORY(gens-sdl)
ENDIF(ENABLE_GENS_SDL)
