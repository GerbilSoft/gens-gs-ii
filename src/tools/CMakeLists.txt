PROJECT(tools)
cmake_minimum_required(VERSION 2.6)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../")

# pot include directory.
INCLUDE_DIRECTORIES(${POPT_INCLUDE_DIR})

ADD_EXECUTABLE(mcd_pcm mcd_pcm.c)
DO_SPLIT_DEBUG(mcd_pcm)
TARGET_LINK_LIBRARIES(mcd_pcm ${POPT_LIBRARY})
IF(WIN32)
	TARGET_LINK_LIBRARIES(mcd_pcm compat_W32U)
ENDIF(WIN32)
