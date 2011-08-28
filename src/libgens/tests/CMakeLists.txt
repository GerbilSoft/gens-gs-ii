PROJECT(libgens-tests)
cmake_minimum_required(VERSION 2.6.0)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")

# TODO: Add a CMake option to enable/disable building test programs.
# I can't seem to find any way to make the test programs a dependency of CTest.

# VdpPalette test programs
ADD_EXECUTABLE(test_VdpPalette_generate test_VdpPalette_generate.c)

ADD_EXECUTABLE(test_VdpPalette test_VdpPalette.cpp)
TARGET_LINK_LIBRARIES(test_VdpPalette gens)

# Generate the test files.
ADD_TEST(NAME test_VdpPalette_generate_MD_Raw
	COMMAND test_VdpPalette_generate --palmode MD --csm raw PalTest_MD_Raw.txt)
ADD_TEST(NAME test_VdpPalette_generate_MD_Full
	COMMAND test_VdpPalette_generate --palmode MD --csm raw PalTest_MD_Full.txt)
ADD_TEST(NAME test_VdpPalette_generate_MD_Full+SH
	COMMAND test_VdpPalette_generate --palmode MD --csm raw PalTest_MD_Full+SH.txt)
ADD_TEST(NAME test_VdpPalette_generate_SMS
	COMMAND test_VdpPalette_generate --palmode SMS PalTest_SMS.txt)
ADD_TEST(NAME test_VdpPalette_generate_GG
	COMMAND test_VdpPalette_generate --palmode GG PalTest_GG.txt)
