PROJECT(libgens)
cmake_minimum_required(VERSION 2.6.0)

# LibGens subprojects.
ADD_SUBDIRECTORY(starscream)
ADD_SUBDIRECTORY(mdZ80)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# LibGens source and binary directories.
# Binary directory is needed for config.h.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# Library checks.
INCLUDE(CheckLibraryExists)
CHECK_LIBRARY_EXISTS(c sigaction "" HAVE_SIGACTION)	# libc: sigaction()
CHECK_LIBRARY_EXISTS(rt clock_gettime "" HAVE_LIBRT)	# librt: clock_gettime()

# Check for iconv() on non-Win32 systems.
IF(WIN32)
	# Win32 has MultiByteToWideChar() and WideCharToMultiByte().
ELSE(WIN32)
	# First, check libc for iconv().
	CHECK_LIBRARY_EXISTS(c iconv "" HAVE_ICONV)	# libc: iconv()
	IF(HAVE_ICONV)
		# iconv() found in libc.
		SET(ICONV_LIBRARY "-lc")
	ELSE(HAVE_ICONV)
		# iconv() not found in libc.
		# Check in libiconv.
		CHECK_LIBRARY_EXISTS(iconv iconv "" HAVE_ICONV)	# libiconv: iconv()
		IF(HAVE_ICONV)
			# iconv() found in libiconv.
			SET(ICONV_LIBRARY "-liconv")
		ENDIF(HAVE_ICONV)
	ENDIF(HAVE_ICONV)
ENDIF(WIN32)

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h")

# Sources.
SET(libgens_SRCS
	lg_main.cpp
	MD/VdpIo.cpp
	MD/VdpRend.cpp
	MD/VdpRend_m5.cpp
	MD/TAB336.c
	MD/EmuMD.cpp
	MD/VdpPalette.cpp
	Save/Zomg.cpp
	cpu/M68K.cpp
	cpu/M68K_Mem.cpp
	sound/Psg.cpp
	sound/Ym2612.cpp
	macros/log_msg.c
	Util/byteswap.c
	Rom.cpp
	Util/siginfo.c
	Util/cpuflags.c
	Effects/CrazyEffect.cpp
	Effects/PausedEffect.cpp
	Effects/FastBlur.cpp
	Util/Timing.cpp
	IO/Io3Button.cpp
	IO/Io6Button.cpp
	IO/Io2Button.cpp
	IO/IoMegaMouse.cpp
	cpu/Z80.cpp
	cpu/Z80_MD_Mem.cpp
	Save/SRam.cpp
	Save/EEPRom.cpp
	IO/IoTeamplayer.cpp
	IO/IoBase.cpp
	credits.c
	lg_osd.c
	Save/EEPRom_File.cpp
	sound/SoundMgr.cpp
	IO/Io4WPMaster.cpp
	IO/Io4WPSlave.cpp
	GensInput/DevManager.cpp
	GensInput/DevManager_KeyNames.cpp
	Decompressor/Decompressor.cpp
	Decompressor/DcGzip.cpp
	Decompressor/DcZip.cpp
	)

IF(HAVE_LZMA)
	SET(libgens_OPTIONAL_SRCS Decompressor/Dc7z.cpp)
ELSE()
	SET(libgens_OPTIONAL_SRCS "")
ENDIF()

IF(HAVE_ICONV)
	SET(libgens_OPTIONAL_SRCS ${libgens_OPTIONAL_SRCS} Util/gens_iconv.c)
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

# Build the library.
ADD_LIBRARY(gens STATIC ${libgens_SRCS} ${libgens_OPTIONAL_SRCS} ${libgens_COMPAT_SRCS})
TARGET_LINK_LIBRARIES(gens starscream mdZ80 minizip)
IF(HAVE_LIBRT)
	TARGET_LINK_LIBRARIES(gens -lrt)
ENDIF(HAVE_LIBRT)
IF(HAVE_ICONV)
	TARGET_LINK_LIBRARIES(gens ${ICONV_LIBRARY})
ENDIF(HAVE_ICONV)
IF(HAVE_LZMA)
	# Link ordering matters:
	# * 7z depends on lzmabase.
	# * lzmabase does NOT depend on 7z.
	TARGET_LINK_LIBRARIES(gens 7z lzmabase)
ENDIF(HAVE_LZMA)

ADD_DEPENDENCIES(gens git_version starscream mdZ80 minizip)
IF(HAVE_LZMA)
	ADD_DEPENDENCIES(lzmabase 7z)
ENDIF(HAVE_LZMA)
