PROJECT(libcompat-tests)
cmake_minimum_required(VERSION 2.6.0)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")

# Google Test.
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR})

# Byteswap test.
ADD_EXECUTABLE(ByteswapTest
	ByteswapTest.cpp
	ByteswapTest.hpp
	ByteswapTest_data.c
	ByteswapTest_data.h
	ByteswapTest_benchmark.cpp
	)
TARGET_LINK_LIBRARIES(ByteswapTest compat ${GTEST_LIBRARY})
ADD_TEST(NAME ByteswapTest
	COMMAND ByteswapTest)
