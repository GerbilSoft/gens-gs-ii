PROJECT(libgens-tests-EEPRomI2CTest)
cmake_minimum_required(VERSION 2.6.0)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")

# Google Test.
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR})

# EEPRomI2C test program.
ADD_EXECUTABLE(EEPRomI2CTest
	EEPRomI2CTest.cpp
	EEPRomI2CTest.hpp
	EEPRomI2CTest_data.c
	EEPRomI2CTest_data.h
	EEPRomI2CTest_SeqRead.cpp
	EEPRomI2CTest_RandomRead.cpp
	)
TARGET_LINK_LIBRARIES(EEPRomI2CTest gens ${ZLIB_LIBRARY} ${GTEST_LIBRARY})
ADD_TEST(NAME EEPRomI2CTest
	COMMAND EEPRomI2CTest)
