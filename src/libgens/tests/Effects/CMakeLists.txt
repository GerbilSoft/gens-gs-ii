PROJECT(libgens-tests-Effects)
cmake_minimum_required(VERSION 2.6.0)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")

# Google Test.
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR})

# Paused Effect Test.
ADD_EXECUTABLE(PausedEffectTest
        PausedEffectTest.cpp
        PausedEffectTest.hpp
        PausedEffectTest_benchmark.cpp
        )
TARGET_LINK_LIBRARIES(PausedEffectTest gens zomg ${GTEST_LIBRARY})
DO_SPLIT_DEBUG(PausedEffectTest)
ADD_TEST(NAME PausedEffectTest
        COMMAND PausedEffectTest)