PROJECT(mdZ80-tests)
cmake_minimum_required(VERSION 2.6.0)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../../")

# Google Test.
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR})

# Z80 instruction tests.
ADD_EXECUTABLE(InsnTests InsnTests.cpp)
TARGET_LINK_LIBRARIES(InsnTests mdZ80 ${GTEST_LIBRARY})
ADD_TEST(NAME mdZ80_InsnTests
	COMMAND InsnTests)
