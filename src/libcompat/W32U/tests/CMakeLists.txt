PROJECT(libcompat_W32U-tests)
cmake_minimum_required(VERSION 2.6.0)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")

# Google Test.
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR})

IF(HAVE_ZLIB AND HAVE_MINIZIP)
	# zlib and MiniZip include directories.
	INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
	INCLUDE_DIRECTORIES(${MINIZIP_INCLUDE_DIR})

	# MiniZip iowin32u test.
	ADD_EXECUTABLE(MiniZip_iowin32u_Test
		MiniZip_iowin32u_Test.cpp
		)
	TARGET_LINK_LIBRARIES(MiniZip_iowin32u_Test compat_W32U_minizip ${MINIZIP_LIBRARY} ${GTEST_LIBRARY})
	DO_SPLIT_DEBUG(MiniZip_iowin32u_Test)
	ADD_TEST(NAME MiniZip_iowin32u_Test
		COMMAND MiniZip_iowin32u_Test)
ENDIF(HAVE_ZLIB AND HAVE_MINIZIP)
