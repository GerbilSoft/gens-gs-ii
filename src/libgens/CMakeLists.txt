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

# sigaction()
CHECK_FUNCTION_EXISTS(sigaction HAVE_SIGACTION)

# clock_gettime() [non-Windows only]
UNSET(RT_LIBRARY)
IF(NOT WIN32)
	CHECK_FUNCTION_EXISTS(clock_gettime HAVE_CLOCK_GETTIME)
	IF(NOT HAVE_CLOCK_GETTIME)
		# clock_gettime() may be in librt.
		CHECK_LIBRARY_EXISTS(rt clock_gettime "" HAVE_CLOCK_GETTIME)
		IF(HAVE_CLOCK_GETTIME)
			SET(HAVE_CLOCK_GETTIME_IN_LIBRT 1)
			SET(RT_LIBRARY rt)
		ENDIF(HAVE_CLOCK_GETTIME)
	ENDIF(NOT HAVE_CLOCK_GETTIME)
ENDIF(NOT WIN32)

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.libgens.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.libgens.h")

# Sources.
SET(libgens_SRCS
	lg_main.cpp
	cpu/M68K.cpp
	cpu/M68K_Mem.cpp
	sound/Psg.cpp
	sound/PsgDebug.cpp
	sound/Ym2612.cpp
	macros/log_msg.c
	Rom.cpp
	Effects/CrazyEffect.cpp
	Effects/PausedEffect.cpp
	Effects/FastBlur.cpp
	cpu/Z80.cpp
	cpu/Z80_MD_Mem.cpp
	Save/SRam.cpp
	credits.c
	lg_osd.c
	sound/SoundMgr.cpp
	sound/SoundMgr_write.cpp
	Data/32X/fw_32x.c
	Cartridge/RomCartridgeMD.cpp
	Save/EEPRomI2C.cpp
	Save/EEPRomI2C_File.cpp
	Save/EEPRomI2C_DB.cpp
	Save/EEPRomI2C_Debug.cpp
	)

SET(libgens_UTIL_SRCS
	Util/gens_siginfo.c
	Util/MdFb.cpp
	Util/Screenshot.cpp
	)

SET(libgens_UTIL_H
	Util/gens_siginfo.h
	Util/MdFb.hpp
	Util/Screenshot.hpp
	)

# OS-specific timing functions.
SET(libgens_TIMING_SRCS Util/Timing.cpp)
SET(libgens_TIMING_H Util/Timing.hpp)
IF(WIN32)
	SET(libgens_TIMING_SRCS ${libgens_TIMING_SRCS} Util/Timing_win32.cpp)
ELSEIF(APPLE)
	SET(libgens_TIMING_SRCS ${libgens_TIMING_SRCS} Util/Timing_mac.cpp)
ELSEIF(UNIX OR LINUX)
	SET(libgens_TIMING_SRCS ${libgens_TIMING_SRCS} Util/Timing_unix.cpp)
ENDIF()

SET(libgens_EMUCONTEXT_SRCS
	EmuContext/EmuContext.cpp
	EmuContext/EmuContextFactory.cpp

	# MD
	EmuContext/EmuMD.cpp
	EmuContext/EmuMD_zomg.cpp
	EmuContext/TmssReg.cpp

	# Pico
	EmuContext/EmuPico.cpp
	EmuContext/EmuPico_zomg.cpp
	)

# TODO: All headers, or just public headers?
SET(libgens_EMUCONTEXT_H
	EmuContext/EmuContext.hpp
	EmuContext/EmuContextFactory.hpp

	# MD
	EmuContext/EmuMD.hpp
	EmuContext/SysVersion.hpp
	EmuContext/TmssReg.hpp

	# Pico
	EmuContext/EmuPico.hpp
	)

SET(libgens_VDP_SRCS
	Vdp/Vdp.cpp
	Vdp/VdpIo.cpp
	Vdp/VdpDma.cpp
	Vdp/VdpRend.cpp
	Vdp/VdpRend_m5.cpp
	Vdp/VdpPalette.cpp
	Vdp/VdpPalette_recalc.cpp
	Vdp/VdpPalette_update.cpp
	Vdp/VdpRend_Err.cpp
	Vdp/VGA_charset.c
	Vdp/VdpStatus.cpp
	Vdp/VdpDebug.cpp
	Vdp/VdpReg.cpp
	Vdp/VdpRend_m4.cpp
	Vdp/VdpRend_tms.cpp
	Vdp/VdpCache.cpp
	)

# TODO: All headers, or just public headers?
SET(libgens_VDP_H
	Vdp/Vdp.hpp
	Vdp/Vdp_p.hpp
	Vdp/VdpPalette.hpp
	Vdp/VdpPalette_p.hpp
	Vdp/VdpRend_Err_p.hpp
	Vdp/VGA_charset.h
	Vdp/VdpStatus.hpp
	Vdp/VdpTypes.hpp
	Vdp/VdpStructs.hpp
	)

SET(libgens_IO_SRCS
	IO/IoManager.cpp
	IO/IoManager_p.cpp
	IO/IoManager_btnNames.cpp
	IO/Device.cpp
	IO/DeviceFactory.cpp
	# Standard controllers
	IO/Io3BTN.cpp
	IO/Io6BTN.cpp
	IO/Io2BTN.cpp
	IO/IoMegaMouse.cpp
	IO/IoXE1AP.cpp
	IO/IoColecoVision.cpp
	IO/IoPico.cpp
	# Multitaps
	IO/IoTeamPlayer.cpp
	IO/Io4WPM.cpp
	IO/Io4WPS.cpp
	IO/IoMasterTap.cpp
	)

# TODO: All headers, or just public headers?
SET(libgens_IO_H
	IO/IoManager.hpp
	IO/IoManager_p.hpp
	IO/Device.hpp
	IO/DeviceFactory.hpp
	# Standard controllers
	IO/Io3BTN.hpp
	IO/Io6BTN.hpp
	IO/Io2BTN.hpp
	IO/IoMegaMouse.hpp
	IO/IoXE1AP.hpp
	IO/IoColecoVision.hpp
	IO/IoPico.hpp
	# Multitaps
	IO/IoTeamPlayer.hpp
	IO/Io4WPM.hpp
	IO/Io4WPS.hpp
	IO/IoMasterTap.hpp
	)

# TODO: Completely rework the decompressor subsystem.
SET(libgens_DECOMPRESSOR_SRCS
	Decompressor/Decompressor.cpp
	Decompressor/DcGzip.cpp
	Decompressor/DcZip.cpp
	Decompressor/DcMemFake.cpp
	)
SET(libgens_DECOMPRESSOR_H
	Decompressor/Decompressor.hpp
	Decompressor/DcGzip.hpp
	Decompressor/DcZip.hpp
	Decompressor/DcMemFake.hpp
	)

# File/Archive subsystem.
SET(libgens_FILE_SRCS
	File/Archive.cpp
	File/ArchiveFactory.cpp
	File/Gzip.cpp
	File/Zip.cpp
	File/MemFake.cpp
	)
SET(libgens_FILE_H
	File/Archive.hpp
	File/ArchiveFactory.hpp
	File/Gzip.hpp
	File/Zip.hpp
	File/MemFake.hpp
	)

IF(HAVE_LZMA)
	SET(libgens_DECOMPRESSOR_SRCS
		${libgens_DECOMPRESSOR_SRCS}
		Decompressor/Dc7z.cpp
		)
	SET(libgens_DECOMPRESSOR_H
		${libgens_DECOMPRESSOR_H}
		Decompressor/Dc7z.hpp
		)
ENDIF()

# Platform-specific sources.
IF(WIN32)
	SET(libgens_DECOMPRESSOR_SRCS
		${libgens_DECOMPRESSOR_SRCS}
		Decompressor/DcRar_Win32.cpp
		Decompressor/UnRAR_dll.cpp
		)
	SET(libgens_DECOMPRESSOR_H
		${libgens_DECOMPRESSOR_H}
		Decompressor/DcRar.hpp
		Decompressor/UnRAR_dll.hpp
		)
ELSE()
	SET(libgens_DECOMPRESSOR_SRCS
		${libgens_DECOMPRESSOR_SRCS}
		Decompressor/DcRar_Unix.cpp
		)
	SET(libgens_DECOMPRESSOR_H
		${libgens_DECOMPRESSOR_H}
		Decompressor/DcRar.hpp
		)
ENDIF()

######################
# Build the library. #
######################

ADD_LIBRARY(gens STATIC
	${libgens_SRCS}
	${libgens_UTIL_SRCS} ${libgens_UTIL_H}
	${libgens_TIMING_SRCS} ${libgens_TIMING_H}
	${libgens_EMUCONTEXT_SRCS} ${libgens_EMUCONTEXT_H}
	${libgens_VDP_SRCS} ${libgens_VDP_H}
	${libgens_IO_SRCS} ${libgens_IO_H}
	${libgens_DECOMPRESSOR_SRCS} ${libgens_DECOMPRESSOR_H}
	${libgens_FILE_SRCS} ${libgens_FILE_H}
	)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(gens)

TARGET_LINK_LIBRARIES(gens compat genstext ${ZLIB_LIBRARY} ${MINIZIP_LIBRARY} zomg)
IF(GENS_ENABLE_EMULATION)
	TARGET_LINK_LIBRARIES(gens starscream mdZ80)
ENDIF(GENS_ENABLE_EMULATION)
IF(HAVE_CLOCK_GETTIME_IN_LIBRT)
	TARGET_LINK_LIBRARIES(gens ${RT_LIBRARY})
ENDIF(HAVE_CLOCK_GETTIME_IN_LIBRT)
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
	TARGET_LINK_LIBRARIES(gens compat_W32U)
	IF(HAVE_MINIZIP)
		# MiniZip Win32 UTF-8 compatibility library.
		TARGET_LINK_LIBRARIES(gens compat_W32U_minizip)
	ENDIF(HAVE_MINIZIP)
ENDIF(WIN32)

# Set the compile definitions.
ADD_DEFINITIONS(${ZLIB_DEFINITIONS} ${MINIZIP_DEFINITIONS})

# Test suite.
IF(BUILD_TESTING)
	ADD_SUBDIRECTORY(tests)
ENDIF(BUILD_TESTING)
