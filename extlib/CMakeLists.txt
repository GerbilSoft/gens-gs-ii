PROJECT(src)
cmake_minimum_required(VERSION 2.6.0)

# External libraries.

# Assembly optimizations.
# TODO: Test on all archs and fix Z_PREFIX issues.
# FIXME: Building 32-bit on 64-bit Linux results in CMAKE_SYSTEM_PROCESSOR == x86_64.
# TODO: If MASM, only enable on cmake-2.6.3+
# TODO: Can't use MASM because of conflicts with NASM in cmake.
# TODO: If nasm/yasm, only enable on cmake-2.8.4+
# TODO: Symbol redefinitions.
IF(0)
IF(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i.|x)86\$")
	SET(ASM686 "ON")
ELSEIF(CMAKE_SYSTEM_PROCESSOR MATCHES "^x86_64\$")
	SET(AMD64 "ON")
ENDIF()
ENDIF()

# Don't install any of these libraries.
SET(OLD_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
SET(OLD_SKIP_INSTALL_LIBRARIES ${SKIP_INSTALL_LIBRARIES})
SET(OLD_SKIP_INSTALL_HEADERS ${SKIP_INSTALL_HEADERS})
SET(OLD_SKIP_INSTALL_FILES ${SKIP_INSTALL_FILES})
SET(OLD_SKIP_INSTALL_ALL ${SKIP_INSTALL_ALL})
SET(BUILD_SHARED_LIBS OFF)
SET(SKIP_INSTALL_LIBRARIES ON)
SET(SKIP_INSTALL_HEADERS ON)
SET(SKIP_INSTALL_FILES ON)
SET(SKIP_INSTALL_ALL ON)

# TODO: If we're using static Qt, check if it has internal zlib/libpng,
# and see if we can use that.

IF(USE_INTERNAL_ZLIB)
	# Use the internal copy of zlib. (statically-linked)
	ADD_SUBDIRECTORY(zlib)
ENDIF(USE_INTERNAL_ZLIB)

IF(USE_INTERNAL_PNG)
	# Use the internal copy of libpng. (statically-linked)
	SET(PNG_SHARED OFF)
	SET(PNG_STATIC ON)
	SET(PNG_TESTS OFF)
	SET(PNG_DEBUG OFF)
	ADD_SUBDIRECTORY(libpng)
ENDIF(USE_INTERNAL_PNG)

# MiniZip
IF(USE_INTERNAL_MINIZIP)
	ADD_SUBDIRECTORY(minizip)
ENDIF(USE_INTERNAL_MINIZIP)

# LZMA
IF(USE_INTERNAL_LZMA)
	ADD_SUBDIRECTORY(lzma)
ENDIF(USE_INTERNAL_LZMA)

IF(USE_INTERNAL_GLEW)
	# Use the internal copy of glew.
	ADD_SUBDIRECTORY(glew)
ENDIF(USE_INTERNAL_GLEW)

# popt
IF(USE_INTERNAL_POPT)
	# Use the internal copy of popt.
	ADD_SUBDIRECTORY(popt)
ENDIF(USE_INTERNAL_POPT)

# Google Test
IF(BUILD_TESTING)
	# Reference: http://stackoverflow.com/questions/12540970/how-to-make-gtest-build-mdd-instead-of-mtd-by-default-using-cmake
	SET(gtest_force_shared_crt ON CACHE BOOL "Always use msvcrt.dll")
	ADD_SUBDIRECTORY(gtest)
ENDIF(BUILD_TESTING)

# FIXME: Probably unnecessary, since these variables
# only apply to the extlib/ subdirectory.
SET(BUILD_SHARED_LIBS ${OLD_BUILD_SHARED_LIBS})
SET(SKIP_INSTALL_LIBRARIES ${OLD_SKIP_INSTALL_LIBRARIES})
SET(SKIP_INSTALL_HEADERS ${OLD_SKIP_INSTALL_HEADERS})
SET(SKIP_INSTALL_FILES ${OLD_SKIP_INSTALL_FILES})
SET(SKIP_INSTALL_ALL ${OLD_SKIP_INSTALL_ALL})
UNSET(OLD_BUILD_SHARED_LIBS)
UNSET(OLD_SKIP_INSTALL_LIBRARIES)
UNSET(OLD_SKIP_INSTALL_HEADERS)
UNSET(OLD_SKIP_INSTALL_FILES)
UNSET(OLD_SKIP_INSTALL_ALL)
