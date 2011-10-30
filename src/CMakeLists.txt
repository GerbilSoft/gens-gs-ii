PROJECT(src)
cmake_minimum_required(VERSION 2.6.0)

# Include the current source and binary directories.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

# Include the extlib directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../extlib/")

# Check for C++ 2011 support.
# If C++ 2011 isn't available, compatibility definitions will be enabled.
INCLUDE(CheckCXXSourceCompiles)
INCLUDE(CheckCXX11CompilerFlag)

SET(SAFE_CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS}")
SET(CMAKE_REQUIRED_DEFINITIONS "${GENS_CXX11_CXXFLAG}")
CHECK_CXX_SOURCE_COMPILES("int main(void) { void *ptr = nullptr; return 0; }" HAVE_CXX_2011)
SET(CMAKE_REQUIRED_DEFINITIONS "${SAFE_CMAKE_REQUIRED_DEFINITIONS}")
UNSET(SAFE_CMAKE_REQUIRED_DEFINITIONS)

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/c++11-compat.h.in" "${CMAKE_CURRENT_BINARY_DIR}/c++11-compat.h")

# Source Code subdirectories.
ADD_SUBDIRECTORY(libzomg)
ADD_SUBDIRECTORY(libgens)
ADD_SUBDIRECTORY(gens-qt4)
ADD_SUBDIRECTORY(tools)
