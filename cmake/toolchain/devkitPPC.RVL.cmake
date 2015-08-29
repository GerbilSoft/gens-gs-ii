# Based on dolphin's toolchain-powerpc.cmake.
# https://github.com/dolphin-emu/hwtests/blob/master/toolchain-powerpc.cmake
SET(DEVKITPRO "$ENV{DEVKITPRO}")
SET(DEVKITPPC "$ENV{DEVKITPPC}")

IF(NOT DEVKITPRO)
	MESSAGE(FATAL_ERROR "Please set DEVKITPRO in your environment.\nexport DEVKITPRO=/path/to/devkitpro")
ENDIF()
IF(NOT EXISTS "${DEVKITPRO}/")
	MESSAGE(FATAL_ERROR "DEVKITPRO is not set to a valid directory.\nDEVKITPRO == ${DEVKITPRO}")
ENDIF(NOT EXISTS "${DEVKITPRO}/")
IF(NOT DEVKITPPC)
	MESSAGE(FATAL_ERROR "Please set DEVKITPPC in your environment.\nexport DEVKITPPC=/path/to/devkitpro/devkitPPC")
ENDIF()
IF(NOT EXISTS "${DEVKITPPC}/")
	MESSAGE(FATAL_ERROR "DEVKITPPC is not set to a valid directory.\nDEVKITPPC == ${DEVKITPPC}")
ENDIF(NOT EXISTS "${DEVKITPPC}/")

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
SET(LIBOGC_INCLUDE_DIR "${DEVKITPRO}/libogc/include")
IF(NOT EXISTS "${LIBOGC_INCLUDE_DIR}/")
	MESSAGE(FATAL_ERROR "libogc include directory not found.\nLIBOGC_INCLUDE_DIR == ${LIBOGC_INCLUDE_DIR}")
ENDIF(NOT EXISTS "${LIBOGC_INCLUDE_DIR}/")
INCLUDE_DIRECTORIES("${LIBOGC_INCLUDE_DIR}")

# GameCube/Wii compiler flags.
# Based on devkitPPC's base_rules, gamecube_rules, and wii_rules.
ADD_DEFINITIONS(-DGEKKO)
SET(DOL_C_FLAGS "-mogc -mcpu=750 -meabi -mhard-float")
SET(RVL_C_FLAGS "-mrvl -mcpu=750 -meabi -mhard-float")

# Enable Wii bulids.
SET(HW_RVL 1)
SET(CMAKE_C_FLAGS "${RVL_C_FLAGS}" CACHE STRING "Flags used by the compiler during all build types." FORCE)
SET(CMAKE_CXX_FLAGS "${RVL_C_FLAGS}" CACHE STRING "Flags used by the compiler during all build types." FORCE)
SET(LIBOGC_PLATFORM "wii")

# Equivalent code for enabling GameCube builds,
# but disabled because we're not targetting GameCube.
IF(0)
SET(HW_DOL 1)
SET(CMAKE_C_FLAGS "${DOL_C_FLAGS}" CACHE STRING "Flags used by the compiler during all build types." FORCE)
SET(CMAKE_CXX_FLAGS "${DOL_C_FLAGS}" CACHE STRING "Flags used by the compiler during all build types." FORCE)
SET(LIBOGC_PLATFORM "cube")
ENDIF(0)

# Libraries to link in.
# NOTE: These must be added in reverse order.
SET(LIBS_TO_LINK "")
SET(LIBOGC_LIBRARY_DIR "${DEVKITPRO}/libogc/lib/${LIBOGC_PLATFORM}")
IF(NOT EXISTS "${LIBOGC_LIBRARY_DIR}/")
	MESSAGE(FATAL_ERROR "libogc not found.\nLIBOGC_LIBRARY_DIR == ${LIBOGC_LIBRARY_DIR}")
ENDIF(NOT EXISTS "${LIBOGC_LIBRARY_DIR}/")
LINK_DIRECTORIES("${LIBOGC_LIBRARY_DIR}")

# Always link in libogc.
SET(LIBOGC 1)
SET(LIBOGC_LIBRARY "${LIBOGC_LIBRARY_DIR}/libogc.a")
IF(NOT EXISTS "${LIBOGC_LIBRARY}")
	MESSAGE(FATAL_ERROR "libogc.a not found.\nLIBOGC_LIBRARY == ${LIBOGC_LIBRARY}")
ENDIF(NOT EXISTS "${LIBOGC_LIBRARY}")
SET(LIBS_TO_LINK "${LIBOGC_LIBRARY} ${LIBS_TO_LINK}")

# Link in bte for Bluetooth. (required by wiiuse)
SET(BTE_LIBRARY "${LIBOGC_LIBRARY_DIR}/libbte.a")
IF(NOT EXISTS "${BTE_LIBRARY}")
	MESSAGE(FATAL_ERROR "libbte.a not found.\nBTE_LIBRARY == ${BTE_LIBRARY}")
ENDIF(NOT EXISTS "${BTE_LIBRARY}")
SET(LIBS_TO_LINK "${BTE_LIBRARY} ${LIBS_TO_LINK}")

# Link in wiiuse for controller input.
SET(WIIUSE_LIBRARY "${LIBOGC_LIBRARY_DIR}/libwiiuse.a")
IF(NOT EXISTS "${WIIUSE_LIBRARY}")
	MESSAGE(FATAL_ERROR "libwiiuse.a not found.\nWIIUSE_LIBRARY == ${WIIUSE_LIBRARY}")
ENDIF(NOT EXISTS "${WIIUSE_LIBRARY}")
SET(LIBS_TO_LINK "${WIIUSE_LIBRARY} ${LIBS_TO_LINK}")

# Set the standard libraries.
SET(CMAKE_C_STANDARD_LIBRARIES "${LIBS_TO_LINK}")
SET(CMAKE_CXX_STANDARD_LIBRARIES "${LIBS_TO_LINK}")
