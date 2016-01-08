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
		UNSET(ICONV_LIBRARY)
		SET(HAVE_ICONV 1)
	ELSE(HAVE_ICONV_C)
		# iconv() not found in libc.
		# Check in libiconv.
		CHECK_LIBRARY_EXISTS(iconv iconv "" HAVE_ICONV_LIBICONV)	# libiconv: iconv()
		IF(HAVE_ICONV_LIBICONV)
			# iconv() found in libiconv.
			SET(ICONV_LIBRARY "iconv")
			SET(HAVE_ICONV 1)
		ENDIF(HAVE_ICONV_LIBICONV)
	ENDIF(HAVE_ICONV_C)
ENDIF(NOT WIN32)

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.libgenstext.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.libgenstext.h")

# Sources.
SET(libgenstext_SRCS
	Encoding.cpp
	StringManip.cpp
	)

######################
# Build the library. #
######################

ADD_LIBRARY(genstext STATIC ${libgenstext_SRCS})
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(genstext)

# Additional libraries.
IF(WIN32)
	TARGET_LINK_LIBRARIES(genstext compat_W32U)
ENDIF(WIN32)
IF(ICONV_LIBRARY)
	TARGET_LINK_LIBRARIES(genstext ${ICONV_LIBRARY})
ENDIF(ICONV_LIBRARY)
