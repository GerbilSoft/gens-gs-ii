PROJECT(libgens-tests)
cmake_minimum_required(VERSION 2.6.0)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")

# Google Test.
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR})

# TestSuite base sources.
SET(TESTSUITE_BASE_SRC
	TestSuite.cpp
	)

# FIXME: Fix this test so it works on MSVC.
IF(NOT MSVC)
# VdpPalette test programs
ADD_EXECUTABLE(test_VdpPalette_DAC_generate test_VdpPalette_DAC_generate.c)

ADD_EXECUTABLE(test_VdpPalette_DAC test_VdpPalette_DAC.cpp ${TESTSUITE_BASE_SRC})
TARGET_LINK_LIBRARIES(test_VdpPalette_DAC gens)

# Generate the test files.
ADD_TEST(NAME test_VdpPalette_DAC_generate_MD
	COMMAND test_VdpPalette_DAC_generate --palmode MD PalTest_MD.txt)
ADD_TEST(NAME test_VdpPalette_DAC_generate_SMS
	COMMAND test_VdpPalette_DAC_generate --palmode SMS PalTest_SMS.txt)
ADD_TEST(NAME test_VdpPalette_DAC_generate_GG
	COMMAND test_VdpPalette_DAC_generate --palmode GG PalTest_GG.txt)

# Test the VDP DAC.
ADD_TEST(NAME test_VdpPalette_DAC_MD
	COMMAND test_VdpPalette_DAC PalTest_MD.txt)
ADD_TEST(NAME test_VdpPalette_DAC_GG
	COMMAND test_VdpPalette_DAC PalTest_GG.txt)
ADD_TEST(NAME test_VdpPalette_DAC_SMS
	COMMAND test_VdpPalette_DAC PalTest_SMS.txt)
ENDIF(NOT MSVC)

# Byteswap test.
ADD_EXECUTABLE(ByteswapTest ByteswapTest.cpp ${TESTSUITE_BASE_SRC})
TARGET_LINK_LIBRARIES(ByteswapTest gens ${GTEST_LIBRARY})
ADD_TEST(NAME ByteswapTest
	COMMAND ByteswapTest)

# Sprite Masking & Overflow Test ROM.
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
ADD_EXECUTABLE(VdpSpriteMaskingTest
	VdpSpriteMaskingTest.cpp
	VdpSpriteMaskingTest_data.c
	${TESTSUITE_BASE_SRC}
	)
TARGET_LINK_LIBRARIES(VdpSpriteMaskingTest gens ${ZLIB_LIBRARY} ${GTEST_LIBRARY})
ADD_TEST(NAME VdpSpriteMaskingTest
	COMMAND VdpSpriteMaskingTest)
