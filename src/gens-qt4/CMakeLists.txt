PROJECT(gens-qt4)
cmake_minimum_required(VERSION 2.6)

# Include the previous directory.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../)

# Sources.
SET(SRCS
	gqt4_main.cpp
	)

# Build the executable.
ADD_EXECUTABLE(gens-qt4 ${SRCS})
TARGET_LINK_LIBRARIES(gens-qt4 gens)
ADD_DEPENDENCIES(gens-qt4 gens)
