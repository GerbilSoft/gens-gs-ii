PROJECT(libgens-tests-Z80Test)
cmake_minimum_required(VERSION 2.6.0)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")

# Google Test.
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR})

IF(GENS_ENABLE_EMULATION)
# Z80 tests.
ADD_EXECUTABLE(Z80Test
	Z80Test.cpp
	)
TARGET_LINK_LIBRARIES(Z80Test z80 ${GTEST_LIBRARY})
# NOTE: zexdoc/zexall isn't working.
#ADD_TEST(NAME Z80Test
#	COMMAND Z80Test)
ENDIF(GENS_ENABLE_EMULATION)
