PROJECT(libgens-tests)
cmake_minimum_required(VERSION 2.6.0)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")

# Google Test.
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR})

# Some tests use ZLIB.
ADD_DEFINITIONS(${ZLIB_DEFINITIONS})

# TestSuite base sources.
SET(TESTSUITE_BASE_SRC
	TestSuite.cpp
	)

# FIXME: Fix this test so it works on MSVC.
IF(NOT MSVC)

# VdpPalette test programs

# test_VdpPalette_DAC_generate runs on the build system,
# not the host system.
# TODO: Port to Google Test.
IF(CMAKE_CROSSCOMPILING)
	# Cross-compiling.
	# Use executables from a native build.
	INCLUDE("${IMPORT_EXECUTABLES}/src/libgens/tests/ImportExecutables.cmake")
ELSE(CMAKE_CROSSCOMPILING)
	# Not cross-compiling.
	ADD_EXECUTABLE(test_VdpPalette_DAC_generate test_VdpPalette_DAC_generate.c)
	DO_SPLIT_DEBUG(test_VdpPalette_DAC_generate)

	# Export these executables for cross-compiling.
	# Reference: http://www.cmake.org/Wiki/CMake_Cross_Compiling
	EXPORT(TARGETS test_VdpPalette_DAC_generate
		FILE "${CMAKE_CURRENT_BINARY_DIR}/ImportExecutables.cmake"
		)
ENDIF(CMAKE_CROSSCOMPILING)

# TODO: Port to Google Test.
ADD_EXECUTABLE(test_VdpPalette_DAC
	test_VdpPalette_DAC.cpp
	${TESTSUITE_BASE_SRC})
TARGET_LINK_LIBRARIES(test_VdpPalette_DAC gens)
DO_SPLIT_DEBUG(test_VdpPalette_DAC)

# NOTE: $<TARGET_FILE:gcbanner> is preferred,
# but this doesn't seem to work on Ubuntu 10.04.
# (cmake_2.8.0-5ubuntu1_i386)
GET_PROPERTY(TEST_VDPPALETTE_DAC_GENERATE_EXE
	TARGET test_VdpPalette_DAC_generate
	PROPERTY LOCATION)

# Generate the test files.
ADD_TEST(NAME test_VdpPalette_DAC_generate_MD
	COMMAND "${TEST_VDPPALETTE_DAC_GENERATE_EXE}" --palmode MD PalTest_MD.txt)
ADD_TEST(NAME test_VdpPalette_DAC_generate_SMS
	COMMAND "${TEST_VDPPALETTE_DAC_GENERATE_EXE}" --palmode SMS PalTest_SMS.txt)
ADD_TEST(NAME test_VdpPalette_DAC_generate_GG
	COMMAND "${TEST_VDPPALETTE_DAC_GENERATE_EXE}" --palmode GG PalTest_GG.txt)

# Test the VDP DAC.
ADD_TEST(NAME test_VdpPalette_DAC_MD
	COMMAND test_VdpPalette_DAC PalTest_MD.txt)
ADD_TEST(NAME test_VdpPalette_DAC_GG
	COMMAND test_VdpPalette_DAC PalTest_GG.txt)
ADD_TEST(NAME test_VdpPalette_DAC_SMS
	COMMAND test_VdpPalette_DAC PalTest_SMS.txt)
ENDIF(NOT MSVC)

# Sprite Masking & Overflow Test ROM.
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
ADD_EXECUTABLE(VdpSpriteMaskingTest
	VdpSpriteMaskingTest.cpp
	VdpSpriteMaskingTest_data.c
	)
TARGET_LINK_LIBRARIES(VdpSpriteMaskingTest compat gens ${ZLIB_LIBRARY} ${GTEST_LIBRARY})
DO_SPLIT_DEBUG(VdpSpriteMaskingTest)
ADD_TEST(NAME VdpSpriteMaskingTest
	COMMAND VdpSpriteMaskingTest)

IF(GENS_ENABLE_EMULATION)
# Z80 tests.
ADD_EXECUTABLE(Z80Tests
	Z80/Z80Tests.cpp
	)
TARGET_LINK_LIBRARIES(Z80Tests mdZ80 ${GTEST_LIBRARY})
DO_SPLIT_DEBUG(Z80Tests)
# NOTE: zexdoc/zexall isn't working.
#ADD_TEST(NAME Z80Tests
#	COMMAND Z80Tests)
ENDIF(GENS_ENABLE_EMULATION)

ADD_SUBDIRECTORY(EEPRomI2CTest)

# VDP FIFO Testing
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
ADD_EXECUTABLE(VdpFIFOTesting
	VdpFIFOTesting.cpp
	VdpFIFOTesting_data.c
	)
TARGET_LINK_LIBRARIES(VdpFIFOTesting compat gens ${GTEST_LIBRARY})
DO_SPLIT_DEBUG(VdpFIFOTesting)
# NOTE: Disabled for now until I can find a way
# to not check tests that are known to fail.
#ADD_TEST(NAME VdpFIFOTesting
#	COMMAND VdpFIFOTesting)

# Sound tests.
ADD_SUBDIRECTORY(sound)
# Effects tests.
ADD_SUBDIRECTORY(Effects)
