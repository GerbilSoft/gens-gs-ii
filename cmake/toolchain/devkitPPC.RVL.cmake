# Based on dolphin's toolchain-powerpc.cmake.
# https://github.com/dolphin-emu/hwtests/blob/master/toolchain-powerpc.cmake
SET(DEVKITPRO "$ENV{DEVKITPRO}")
SET(DEVKITPPC "$ENV{DEVKITPPC}")

IF(NOT DEVKITPRO)
	MESSAGE(FATAL_ERROR "Please set DEVKITPRO in your environment.\nexport DEVKITPRO=/path/to/devkitpro")
ENDIF()
IF(NOT DEVKITPPC)
	MESSAGE(FATAL_ERROR "Please set DEVKITPPC in your environment.\nexport DEVKITPPC=/path/to/devkitpro/devkitPPC")
ENDIF()

SET(CMAKE_SYSTEM_NAME "Generic")
SET(CMAKE_SYSTEM_VERSION 0)
SET(CMAKE_SYSTEM_PROCESSOR powerpc-eabi)

SET(CMAKE_SYSROOT "${DEVKITPPC}")
#SET(CMAKE_STAGING_PREFIX)

# TODO: Append .exe on Windows build systems.
SET(CMAKE_C_COMPILER "${CMAKE_SYSROOT}/bin/powerpc-eabi-gcc")
SET(CMAKE_CXX_COMPILER "${CMAKE_SYSROOT}/bin/powerpc-eabi-g++")
SET(CMAKE_AR "${CMAKE_SYSROOT}/bin/powerpc-eabi-ar" CACHE FILEPATH "ar")
SET(CMAKE_NM "${CMAKE_SYSROOT}/bin/powerpc-eabi-nm" CACHE FILEPATH "nm")
SET(CMAKE_RANLIB "${CMAKE_SYSROOT}/bin/powerpc-eabi-ranlib" CACHE FILEPATH "ranlib")

# NOTE: System libraries are in the CPU-specific subdirectory.
# ${DEVKITPPC}/lib only has gcc's libiberty and startup code.
SET(CMAKE_FIND_ROOT_PATH "${DEVKITPPC}/${CMAKE_SYSTEM_PROCESSOR}")
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# libOGC directories.
INCLUDE_DIRECTORIES("$ENV{DEVKITPRO}/libogc/include")
LINK_DIRECTORIES("$ENV{DEVKITPRO}/libogc/lib/wii")

# Wii-specific flags.
# Based on devkitPPC's base_rules and wii_rules.
ADD_DEFINITIONS(-DGEKKO)
SET(RVL_C_FLAGS "-mrvl -mcpu=750 -meabi -mhard-float")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${RVL_C_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RVL_C_FLAGS}")

# Always link in libogc.
# NOTE: LINK_DIRECTORIES() isn't working...
SET(LIBOGC_LIBRARY "$ENV{DEVKITPRO}/libogc/lib/wii/libogc.a" CACHE FILEPATH "libogc")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LIBOGC_LIBRARY}")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LIBOGC_LIBRARY}")
