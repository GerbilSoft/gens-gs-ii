PROJECT(libgenstext)
cmake_minimum_required(VERSION 2.6.0)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../")

# LibGens source and binary directories.
# Binary directory is needed for config.h.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# On Win32, we use MultiByteToWideChar() and WideCharToMultiByte().
# On other systems, we use iconv(), which could be in libc or libiconv.
# Figure out which library has iconv().
IF(NOT WIN32)
	# First, check libc for iconv().
	CHECK_LIBRARY_EXISTS(c iconv "" HAVE_ICONV_C)	# libc: iconv()
	IF(HAVE_ICONV_C)
		# iconv() found in libc.
		SET(ICONV_LIBRARY "-lc")
		SET(HAVE_ICONV 1)
	ELSE(HAVE_ICONV_C)
		# iconv() not found in libc.
		# Check in libiconv.
		CHECK_LIBRARY_EXISTS(iconv iconv "" HAVE_ICONV_LIBICONV)	# libiconv: iconv()
		IF(HAVE_ICONV_LIBICONV)
			# iconv() found in libiconv.
			SET(ICONV_LIBRARY "-liconv")
			SET(HAVE_ICONV 1)
		ENDIF(HAVE_ICONV_LIBICONV)
	ENDIF(HAVE_ICONV_C)
ENDIF(NOT WIN32)

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.libgenstext.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.libgenstext.h")

# Test system byteorder.
# TODO: Universal Binary support; detect PDP endian.
INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
IF(IS_BIG_ENDIAN)
	SET(GENSTEXT_BYTEORDER "GENSTEXT_BIG_ENDIAN")
ELSE(IS_BIG_ENDIAN)
	SET(GENSTEXT_BYTEORDER "GENSTEXT_LIL_ENDIAN")
ENDIF(IS_BIG_ENDIAN)
UNSET(IS_BIG_ENDIAN)
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/byteorder.h.in" "${CMAKE_CURRENT_BINARY_DIR}/byteorder.h")

# Sources.
SET(libgenstext_SRCS
	Encoding.cpp
	StringManip.cpp
	)

######################
# Build the library. #
######################

SET(LIBGENSTEXT_PDB_FILE "${CMAKE_CURRENT_BINARY_DIR}/genstext.pdb")

IF(MSVC)
	# CMake seems to use weird settings for the PDB file.
	# (at least version 2.8.12.2; 3.0.0 might be different)
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_C}"
		"/Fd${LIBGENSTEXT_PDB_FILE}"
		CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT}")
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_CXX}"
		"/Fd${LIBGENSTEXT_PDB_FILE}"
		CMAKE_CXX_COMPILE_OBJECT "${CMAKE_CXX_COMPILE_OBJECT}")
ENDIF(MSVC)

ADD_LIBRARY(genstext STATIC ${libgenstext_SRCS})
