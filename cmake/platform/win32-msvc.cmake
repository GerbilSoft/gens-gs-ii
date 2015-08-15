# Win32-specific CFLAGS/CXXFLAGS.
# For Microsoft Visual C++ compilers.

# Basic platform flags:
# - wchar_t should be a distinct type.
# - Enable strict type checking in the Windows headers.
# - Define WIN32_LEAN_AND_MEAN to reduce the number of Windows headers included.
# - Define NOMINMAX to disable the MIN() and MAX() macros.
SET(GENS_C_FLAGS_WIN32 "-Zc:wchar_t -DSTRICT -DWIN32_LEAN_AND_MEAN -DNOMINMAX")

# Disable ANSI Windows support if:
# - We're building for 64-bit.
# - We're using MSVC 2008 or later.
IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
	# 64-bit build.
	MESSAGE(WARNING "Building for 64-bit; disabling ANSI Windows support.")
	SET(ENABLE_ANSI_WINDOWS "OFF")
ELSEIF(MSVC_VERSION GREATER 1400)
	# MSVC 2008 (9.0, 1500) requires a minimum subsystem
	# version of 5.00 for the Windows and console subsystems.
	MESSAGE(WARNING "MSVC 2008 and later do not support targetting versions of Windows earlier than Windows 2000.\nANSI Windows support has been disabled.")
	SET(ENABLE_ANSI_WINDOWS "OFF")
ENDIF()

# Subsystem and minimum Windows version:
# - If 32-bit and ANSI Windows support is enabled: 4.00
# - If 32-bit and ANSI Windows support is disabled: 5.00
# - If 64-bit: 5.02
# NOTE: MSVC 2008 and 2010 have a minimum subsystem value of 4.00.
# NOTE: MSVC 2012 and later have a minimum subsystem value of 5.01.
# NOTE: CMAKE sets /subsystem:windows or /subsystem:console itself.
# It does not affect the subsystem version that's set here, though.
IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
	# 64-bit, Unicode Windows only.
	# (There is no 64-bit ANSI Windows.)
	SET(GENS_C_FLAGS_WIN32 "${GENS_C_FLAGS_WIN32} -D_WIN32_WINNT=0x0502")
	SET(GENS_EXE_LINKER_FLAGS_WIN32 "-subsystem:windows,5.02")
ELSEIF(ENABLE_ANSI_WINDOWS)
	# 32-bit, ANSI Windows support enabled.
	SET(GENS_C_FLAGS_WIN32 "${GENS_C_FLAGS_WIN32} -D_WIN32_WINNT=0x0400")
	SET(GENS_EXE_LINKER_FLAGS_WIN32 "-subsystem:windows,4.00")
ELSE()
	# 32-bit, Unicode Windows only.
	SET(GENS_C_FLAGS_WIN32 "${GENS_C_FLAGS_WIN32} -D_WIN32_WINNT=0x0500")
	IF(MSVC_VERSION GREATER 1600)
		# MSVC 2012 or later.
		# Minimum target version is Windows XP.
		SET(GENS_EXE_LINKER_FLAGS_WIN32 "-subsystem:windows,5.01")
	ELSE()
		# MSVC 2010 or earlier.
		SET(GENS_EXE_LINKER_FLAGS_WIN32 "-subsystem:windows,5.00")
	ENDIF()
ENDIF()
SET(GENS_SHARED_LINKER_FLAGS_WIN32 "${GENS_EXE_LINKER_FLAGS_WIN32}")

# Release build: Prefer static libraries.
IF(CMAKE_BUILD_TYPE MATCHES ^release)
	SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ENDIF(CMAKE_BUILD_TYPE MATCHES ^release)

# Append the CFLAGS and LDFLAGS.
SET(GENS_C_FLAGS_COMMON			"${GENS_C_FLAGS_COMMON} ${GENS_C_FLAGS_WIN32}")
SET(GENS_CXX_FLAGS_COMMON		"${GENS_CXX_FLAGS_COMMON} ${GENS_C_FLAGS_WIN32}")
SET(GENS_EXE_LINKER_FLAGS_COMMON	"${GENS_EXE_LINKER_FLAGS_COMMON} ${GENS_EXE_LINKER_FLAGS_WIN32}")
SET(GENS_SHARED_LINKER_FLAGS_COMMON	"${GENS_SHARED_LINKER_FLAGS_COMMON} ${GENS_EXE_LINKER_FLAGS_WIN32}")

# Unset temporary variables.
UNSET(GENS_C_FLAGS_WIN32)
UNSET(GENS_EXE_LINKER_FLAGS_WIN32)
