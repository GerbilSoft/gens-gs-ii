PROJECT(libgens-tests-Effects)
cmake_minimum_required(VERSION 2.6.0)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")

# Google Test.
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR})

# Copy the reference images to the test directory.
FILE(COPY
	Effects.Normal.15.png
	Effects.Normal.16.png
	Effects.Normal.32.png
	PausedEffect.SW-int.15.png
	PausedEffect.SW-int.16.png
	PausedEffect.SW-int.32.png
	FastBlur.SW.15.png
	FastBlur.SW.16.png
	FastBlur.SW.32.png
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
	)

# Paused Effect Test.
ADD_EXECUTABLE(PausedEffectTest
	EffectTest.cpp
	EffectTest.hpp
        PausedEffectTest.cpp
        PausedEffectTest.hpp
        PausedEffectTest_benchmark.cpp
        )
TARGET_LINK_LIBRARIES(PausedEffectTest gens zomg ${GTEST_LIBRARY})
DO_SPLIT_DEBUG(PausedEffectTest)
ADD_TEST(NAME PausedEffectTest
        COMMAND PausedEffectTest)

# Fast Blur Test.
ADD_EXECUTABLE(FastBlurTest
	EffectTest.cpp
	EffectTest.hpp
        FastBlurTest.cpp
        FastBlurTest.hpp
        FastBlurTest_benchmark.cpp
        )
TARGET_LINK_LIBRARIES(FastBlurTest gens zomg ${GTEST_LIBRARY})
DO_SPLIT_DEBUG(FastBlurTest)
ADD_TEST(NAME FastBlurTest
        COMMAND FastBlurTest)
