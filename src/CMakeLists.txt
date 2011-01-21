PROJECT(src)
cmake_minimum_required(VERSION 2.6.0)

# TODO: Make 7-Zip support optional.
SET(HAVE_LZMA 1)

# External libraries.
ADD_SUBDIRECTORY(extlib)

# Add the extlib/ directory to the includes list.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/extlib/")

# Source Code subdirectories.
ADD_SUBDIRECTORY(libzomg)
ADD_SUBDIRECTORY(libgens)
ADD_SUBDIRECTORY(gens-qt4)
ADD_SUBDIRECTORY(tools)
