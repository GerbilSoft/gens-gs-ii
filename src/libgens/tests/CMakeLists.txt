PROJECT(libgens-tests)
cmake_minimum_required(VERSION 2.6.0)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")

# VdpPalette
ADD_EXECUTABLE(test_VdpPalette_generate test_VdpPalette_generate.c)
# TODO: VdpPalette test program.
# TODO: Run the generator and tester.
