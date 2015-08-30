PROJECT(libzomg-tests)
cmake_minimum_required(VERSION 2.6.0)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")

# Google Test.
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR})

# Byteswap test.
ADD_EXECUTABLE(PrintMetadata
	PrintMetadata.cpp
	)
TARGET_LINK_LIBRARIES(PrintMetadata zomg gens)
DO_SPLIT_DEBUG(PrintMetadata)
# NOTE: This isn't a standard test suite.
# It prints the contents of what ZOMG.ini
# would contain in a savestate.
#ADD_TEST(NAME PrintMetadata
#	COMMAND PrintMetadata)
