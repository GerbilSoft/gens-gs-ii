PROJECT(src)
cmake_minimum_required(VERSION 2.6.0)

# Include the extlib directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../extlib/")

# Source Code subdirectories.
ADD_SUBDIRECTORY(libzomg)
ADD_SUBDIRECTORY(libgens)
ADD_SUBDIRECTORY(gens-qt4)
ADD_SUBDIRECTORY(tools)
