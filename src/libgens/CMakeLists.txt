PROJECT(libgens)
cmake_minimum_required(VERSION 2.6.0)

# LibGens subprojects.
IF(GENS_ENABLE_EMULATION)
	ADD_SUBDIRECTORY(starscream)
	ADD_SUBDIRECTORY(mdZ80)
ENDIF(GENS_ENABLE_EMULATION)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../")

# LibGens source and binary directories.
# Binary directory is needed for config.h.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# ZLIB and MINIZIP include directories.
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${MINIZIP_INCLUDE_DIR})

# Library checks.
INCLUDE(CheckLibraryExists)
CHECK_LIBRARY_EXISTS(c sigaction "" HAVE_SIGACTION)	# libc: sigaction()

# FIXME: Non-Windows only?
CHECK_LIBRARY_EXISTS(rt clock_gettime "" HAVE_LIBRT)	# librt: clock_gettime()
IF(HAVE_LIBRT)
	SET(RT_LIBRARY rt)
ENDIF(HAVE_LIBRT)

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.libgens.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.libgens.h")

# Test system byteorder.
# TODO: Universal Binary support; detect PDP endian.
INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
IF(IS_BIG_ENDIAN)
	SET(GENS_BYTEORDER "GENS_BIG_ENDIAN")
ELSE(IS_BIG_ENDIAN)
	SET(GENS_BYTEORDER "GENS_LIL_ENDIAN")
ENDIF(IS_BIG_ENDIAN)
UNSET(IS_BIG_ENDIAN)
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/Util/byteorder.h.in" "${CMAKE_CURRENT_BINARY_DIR}/Util/byteorder.h")

# Sources.
SET(libgens_SRCS
	lg_main.cpp
	MD/EmuMD.cpp
	Save/GensZomg.cpp
	cpu/M68K.cpp
	cpu/M68K_Mem.cpp
	sound/Psg.cpp
	sound/Ym2612.cpp
	macros/log_msg.c
	Util/byteswap.c
	Rom.cpp
	Util/gens_siginfo.c
	Util/cpuflags.c
	Effects/CrazyEffect.cpp
	Effects/PausedEffect.cpp
	Effects/FastBlur.cpp
	Util/Timing.cpp
	cpu/Z80.cpp
	cpu/Z80_MD_Mem.cpp
	Save/SRam.cpp
	Save/EEPRom.cpp
	credits.c
	lg_osd.c
	Save/EEPRom_File.cpp
	sound/SoundMgr.cpp
	Decompressor/Decompressor.cpp
	Decompressor/DcGzip.cpp
	Decompressor/DcZip.cpp
	EmuContext.cpp
	Save/EEPRom_DB.cpp
	Data/mcd_rom_db.c
	Data/32X/fw_32x.c
	Util/MdFb.cpp
	IO/IoManager.cpp
	IO/IoManager_p.cpp
	IO/IoManager_p_update.cpp
	IO/IoManager_btnNames.cpp
	Cartridge/RomCartridgeMD.cpp
	MD/TmssReg.cpp
	)

SET(libgens_VDP_SRCS
	Vdp/Vdp.cpp
	Vdp/VdpIo.cpp
	Vdp/VdpRend.cpp
	Vdp/VdpRend_m5.cpp
	Vdp/VdpPalette.cpp
	Vdp/VdpRend_Err.cpp
	Vdp/VGA_charset.c
	Vdp/VdpStatus.cpp
	Vdp/VdpDebug.cpp
	Vdp/VdpReg.cpp
	Vdp/VdpRend_m4.cpp
	)

# TODO: All headers, or just public headers?
SET(libgens_VDP_H
	Vdp/Vdp.hpp
	Vdp/Vdp_p.hpp
	Vdp/VdpPalette.hpp
	Vdp/VdpRend_Err_p.hpp
	Vdp/VGA_charset.h
	Vdp/VdpStatus.hpp
	Vdp/VdpTypes.hpp
	Vdp/VdpStructs.hpp
	)

IF(HAVE_LZMA)
	SET(libgens_OPTIONAL_SRCS Decompressor/Dc7z.cpp)
ELSE()
	SET(libgens_OPTIONAL_SRCS "")
ENDIF()

# Platform-specific sources.
IF(WIN32)
	SET(libgens_COMPAT_SRCS
		Decompressor/DcRar_Win32.cpp
		Decompressor/UnRAR_dll.cpp
		Win32/W32U_mini.c
		)
ELSE()
	SET(libgens_COMPAT_SRCS Decompressor/DcRar_Unix.cpp)
ENDIF()

######################
# Build the library. #
######################

ADD_LIBRARY(gens STATIC
	${libgens_SRCS}
	${libgens_VDP_SRCS}
	${libgens_VDP_H}
	${libgens_OPTIONAL_SRCS}
	${libgens_COMPAT_SRCS}
	)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(gens)

TARGET_LINK_LIBRARIES(gens genstext ${ZLIB_LIBRARY} ${MINIZIP_LIBRARY} zomg)
IF(GENS_ENABLE_EMULATION)
	TARGET_LINK_LIBRARIES(gens starscream mdZ80)
ENDIF(GENS_ENABLE_EMULATION)
IF(HAVE_LIBRT)
	TARGET_LINK_LIBRARIES(gens ${RT_LIBRARY})
ENDIF(HAVE_LIBRT)
IF(HAVE_ICONV)
	TARGET_LINK_LIBRARIES(gens ${ICONV_LIBRARY})
ENDIF(HAVE_ICONV)
IF(HAVE_LZMA)
	# Link ordering matters:
	# * 7z depends on lzmabase.
	# * lzmabase does NOT depend on 7z.
	TARGET_LINK_LIBRARIES(gens ${7z_LIBRARY} ${LZMA_LIBRARY})
ENDIF(HAVE_LZMA)
IF(WIN32)
	# version.dll is required for DcRar_Win32.cpp, which calls these functions:
	# - GetFileVersionInfoSize()
	# - GetFileVersionInfo()
	# - VerQueryValue()
	TARGET_LINK_LIBRARIES(gens version)
ENDIF(WIN32)

# Test suite.
IF(BUILD_TESTING)
	ADD_SUBDIRECTORY(tests)
ENDIF(BUILD_TESTING)
