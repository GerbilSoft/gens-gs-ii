PROJECT(libgens-tests)
cmake_minimum_required(VERSION 2.6.0)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")

# VdpPalette test programs
ADD_EXECUTABLE(test_VdpPalette_DAC_generate test_VdpPalette_DAC_generate.c)

ADD_EXECUTABLE(test_VdpPalette_DAC test_VdpPalette_DAC.cpp TestSuite.cpp)
TARGET_LINK_LIBRARIES(test_VdpPalette_DAC gens)

# Generate the test files.
ADD_TEST(NAME test_VdpPalette_DAC_generate_MD_Raw
	COMMAND test_VdpPalette_DAC_generate --palmode MD --csm raw PalTest_MD_Raw.txt)
ADD_TEST(NAME test_VdpPalette_DAC_generate_MD_Full
	COMMAND test_VdpPalette_DAC_generate --palmode MD --csm full PalTest_MD_Full.txt)
ADD_TEST(NAME test_VdpPalette_DAC_generate_MD_Full+SH
	COMMAND test_VdpPalette_DAC_generate --palmode MD --csm full+sh PalTest_MD_Full+SH.txt)
ADD_TEST(NAME test_VdpPalette_DAC_generate_SMS
	COMMAND test_VdpPalette_DAC_generate --palmode SMS PalTest_SMS.txt)
ADD_TEST(NAME test_VdpPalette_DAC_generate_GG
	COMMAND test_VdpPalette_DAC_generate --palmode GG PalTest_GG.txt)
