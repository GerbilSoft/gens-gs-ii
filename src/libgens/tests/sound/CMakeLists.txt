PROJECT(libgens-tests-sound)
cmake_minimum_required(VERSION 2.6.0)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")

# Google Test.
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR})

# PSG Register Test.
ADD_EXECUTABLE(PsgRegisterTest
        PsgRegisterTest.cpp
        )
TARGET_LINK_LIBRARIES(PsgRegisterTest gens ${GTEST_LIBRARY})
ADD_TEST(NAME PsgRegisterTest
        COMMAND PsgRegisterTest)

# Audio Write Test.
# TODO: Generate the data file?
ADD_EXECUTABLE(AudioWriteTest
        AudioWriteTest_data.c
        AudioWriteTest.cpp
        AudioWriteTest_benchmark.cpp
        )
TARGET_LINK_LIBRARIES(AudioWriteTest gens ${GTEST_LIBRARY})
ADD_TEST(NAME AudioWriteTest
        COMMAND AudioWriteTest)
