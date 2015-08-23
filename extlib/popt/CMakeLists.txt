PROJECT(popt C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)

# popt source and binary directories.
# Binary directory is needed for config.h.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

SET(VERSION "1.16")

INCLUDE(CheckFunctionExists)
INCLUDE(CheckIncludeFile)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckSymbolExists)

# Disable NLS.
# We're not including the .po files.
SET(ENABLE_NLS 0)

### configure.ac ###

# Package information.
SET(PACKAGE "popt")
SET(PACKAGE_NAME "popt")
SET(PACKAGE_TARNAME "popt")
SET(PACKAGE_VERSION "${VERSION}")
SET(PACKAGE_STRING "popt ${VERSION}")
SET(PACKAGE_BUGREPORT "popt-devel@rpm5.org")

# Check for various functions.
CHECK_FUNCTION_EXISTS(getuid		HAVE_GETUID)
CHECK_FUNCTION_EXISTS(geteuid		HAVE_GETEUID)
CHECK_FUNCTION_EXISTS(iconv		HAVE_ICONV)
CHECK_FUNCTION_EXISTS(mtrace		HAVE_MTRACE)
CHECK_FUNCTION_EXISTS(secure_getenv	HAVE_SECURE_GETENV)
IF(NOT HAVE_SECURE_GETENV)
	CHECK_FUNCTION_EXISTS(__secure_getenv	HAVE___SECURE_GETENV)
ENDIF(NOT HAVE_SECURE_GETENV)
CHECK_FUNCTION_EXISTS(setregid		HAVE_SETREGID)
CHECK_FUNCTION_EXISTS(stpcpy		HAVE_STPCPY)
CHECK_FUNCTION_EXISTS(strerror		HAVE_STRERROR)
CHECK_FUNCTION_EXISTS(vasprintf		HAVE_VASPRINTF)
CHECK_FUNCTION_EXISTS(srandom		HAVE_SRANDOM)

# setreuid() may be in libucb on some systems.
CHECK_FUNCTION_EXISTS(setreuid		HAVE_SETREUID)
IF(NOT HAVE_SETREUID)
	CHECK_LIBRARY_EXISTS(ucb setreuid "" HAVE_LIBUCB)
	IF(HAVE_LIBUCB)
		SET(HAVE_SETREUID 1 CACHE INTERNAL "Have function setreuid (in libucb)")
		TARGET_LINK_LIBRARIES(popt ucb)
	ENDIF(HAVE_LIBUCB)
ENDIF(NOT HAVE_SETREUID)

# Check for header files.
CHECK_INCLUDE_FILE(float.h	HAVE_FLOAT_H)
CHECK_INCLUDE_FILE(fnmatch.h	HAVE_FNMATCH_H)
CHECK_INCLUDE_FILE(glob.h	HAVE_GLOB_H)
CHECK_INCLUDE_FILE(langinfo.h	HAVE_LANGINFO_H)
CHECK_INCLUDE_FILE(libintl.h	HAVE_LIBINTL_H)
CHECK_INCLUDE_FILE(mcheck.h	HAVE_MCHECK_H)
CHECK_INCLUDE_FILE(unistd.h	HAVE_UNISTD_H)

# Other checks not provided by configure.ac.
# (Automatic autoconf checks?)
CHECK_FUNCTION_EXISTS(dcgettext		HAVE_DCGETTEXT)
CHECK_FUNCTION_EXISTS(gettext		HAVE_GETTEXT)

CHECK_INCLUDE_FILE(dlfcn.h	HAVE_DLFCN_H)
CHECK_INCLUDE_FILE(inttypes.h	HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(memory.h	HAVE_MEMORY_H)
CHECK_INCLUDE_FILE(stdint.h	HAVE_STDINT_H)
CHECK_INCLUDE_FILE(stdlib.h	HAVE_STDLIB_H)
CHECK_INCLUDE_FILE(strings.h	HAVE_STRINGS_H)
CHECK_INCLUDE_FILE(sys/stat.h	HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE(sys/types.h	HAVE_SYS_TYPES_H)

# We're always assuming these are true.
SET(PROTOTYPES 1)
SET(STDC_HEADERS 1)

# Additional checks for Windows compatibility.
CHECK_INCLUDE_FILE(sys/ioctl.h	HAVE_SYS_IOCTL_H)
IF(HAVE_SYS_IOCTL_H)
	CHECK_SYMBOL_EXISTS(TIOCGWINSZ sys/ioctl.h HAVE_TIOCGWINSZ)
ENDIF(HAVE_SYS_IOCTL_H)

# NOTE: This should not be used on Windows...
SET(POPT_SYSCONFDIR "${CMAKE_INSTALL_PREFIX}/etc")

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h")
ADD_DEFINITIONS(-DHAVE_CONFIG_H)

# Source files.
SET(popt_SRCS
	popt.c
	poptconfig.c
	popthelp.c
	poptint.c
	poptparse.c
	)

# Internal header files.
SET(popt_H_INT
	poptint.h
	system.h
	)

# Public header files.
SET(popt_H_PUBLIC
	popt.h
	)

######################
# Build the library. #
######################

ADD_LIBRARY(popt STATIC
	${popt_SRCS}
	${popt_H_INT}
	${popt_H_PUBLIC}
	)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(popt)
