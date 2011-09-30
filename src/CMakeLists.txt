PROJECT(src)
cmake_minimum_required(VERSION 2.6.0)

# TODO: Make 7-Zip support optional.
# TODO: Set LZMA_FOUND if we're using an external LZMA.
# (That is, if we ever decide to add support for external LZMA.)
SET(HAVE_LZMA 1)
#SET(LZMA_FOUND 1)

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

# External libraries.
ADD_SUBDIRECTORY(extlib)

# Add the extlib/ directory to the includes list.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/extlib/")

# Source Code subdirectories.
ADD_SUBDIRECTORY(libzomg)
ADD_SUBDIRECTORY(libgens)
ADD_SUBDIRECTORY(gens-qt4)
ADD_SUBDIRECTORY(tools)
