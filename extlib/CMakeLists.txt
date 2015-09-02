PROJECT(src)
cmake_minimum_required(VERSION 2.6.0)

# External libraries.

# Assembly optimizations.
# TODO: Test on all archs and fix Z_PREFIX issues.
# TODO: Add more assembly files from contrib/ besides the
# default ones that were listed in zlib's CMakeLists.txt.
# TODO: If MASM, only enable on cmake-2.6.3+
# TODO: If nasm/yasm, only enable on cmake-2.8.4+
# cmake_minimum_required() statements have been added to zlib's
# CMakeLists.txt to prevent errors, but ideally, we should be
# able to simply disable asm on older versions.
STRING(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" arch)
IF(arch MATCHES "^(i.|x)86$|^x86_64$|^amd64$")
	# i386/amd64. Check sizeof(void*) for the actual architecture,
	# since building 32-bit on 64-bit isn't considered "cross-compiling",
	# so CMAKE_SYSTEM_PROCESSOR might not be accurate.
	# NOTE: CMake sometimes leaves this blank for MSVC builds,
	# so check for 64-bit first so we can default to 32-bit.
	IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
		SET(AMD64 "ON")
	ELSE()
		SET(ASM686 "ON")
	ENDIF()
ENDIF(arch MATCHES "^(i.|x)86$|^x86_64$|^amd64$")
UNSET(arch)

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
