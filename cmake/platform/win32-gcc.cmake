# Win32-specific CFLAGS/CXXFLAGS.
# For MinGW compilers.

# Basic platform flags:
# - wchar_t is short.
# - Enable strict type checking in the Windows headers.
# - Set minimum Windows version to Windows 2000. (Windows NT 5.0)
# - Define WIN32_LEAN_AND_MEAN to reduce the number of Windows headers included.
# - Define NOMINMAX to disable the MIN() and MAX() macros.
SET(GENS_C_FLAGS_WIN32 "-fshort-wchar -DSTRICT -DWIN32_LEAN_AND_MEAN -DNOMINMAX")

# Subsystem and minimum Windows version:
# - If 32-bit and ANSI Windows support is enabled: 4.00
# - If 32-bit and ANSI Windows support is disabled: 5.00
# - If 64-bit: 5.02
IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
	# 64-bit, Unicode Windows only.
	# (There is no 64-bit ANSI Windows.)
	SET(GENS_C_FLAGS_WIN32 "${GENS_C_FLAGS_WIN32} -D_WIN32_WINNT=0x0502")
	SET(GENS_EXE_LINKER_FLAGS_WIN32 "--subsystem windows:5.02")
ELSEIF(${ENABLE_ANSI_WINDOWS} EQUAL "ON")
	# TODO: Enable CMP0012? (requires cmake-2.8.0)
	# 32-bit, ANSI Windows support enabled.
	SET(GENS_C_FLAGS_WIN32 "${GENS_C_FLAGS_WIN32} -D_WIN32_WINNT=0x0400")
	SET(GENS_EXE_LINKER_FLAGS_WIN32 "--subsystem windows:4.00")
ELSE()
	# 32-bit, Unicode Windows only.
	SET(GENS_C_FLAGS_WIN32 "${GENS_C_FLAGS_WIN32} -D_WIN32_WINNT=0x0500")
	SET(GENS_EXE_LINKER_FLAGS_WIN32 "--subsystem windows:5.00")
ENDIF()
SET(GENS_SHARED_LINKER_FLAGS_WIN32 "${GENS_EXE_LINKER_FLAGS_WIN32}")

# Release build: Prefer static libraries.
IF(CMAKE_BUILD_TYPE MATCHES ^release)
	SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ENDIF(CMAKE_BUILD_TYPE MATCHES ^release)

# Test for static libgcc/libstdc++.
# NOTE: Technically, --tsaware is only valid for EXEs, not DLLs,
# but we should mark DLLs anyway.
FOREACH(FLAG_TEST "-static-libgcc" "-static-libstdc++" "-Wl,--large-address-aware" "-Wl,--nxcompat" "-Wl,--dynamicbase" "-Wl,--tsaware")
	# CMake doesn't like "+" characters in variable names.
	STRING(REPLACE "+" "_" FLAG_TEST_VARNAME "${FLAG_TEST}")

	CHECK_C_COMPILER_FLAG("${FLAG_TEST}" LDFLAG_${FLAG_TEST_VARNAME})
	IF(LDFLAG_${FLAG_TEST_VARNAME})
		SET(GENS_EXE_LINKER_FLAGS_WIN32 "${GENS_EXE_LINKER_FLAGS_WIN32} ${FLAG_TEST}")
	ENDIF(LDFLAG_${FLAG_TEST_VARNAME})
	UNSET(LDFLAG_${FLAG_TEST_VARNAME})
	UNSET(FLAG_TEST_VARNAME)
ENDFOREACH()

# Enable windres support on MinGW.
# http://www.cmake.org/Bug/view.php?id=4068
SET(CMAKE_RC_COMPILER_INIT windres)
ENABLE_LANGUAGE(RC)

# NOTE: Setting CMAKE_RC_OUTPUT_EXTENSION doesn't seem to work.
# Force windres to output COFF, even though it'll use the .res extension.
SET(CMAKE_RC_OUTPUT_EXTENSION .obj)
SET(CMAKE_RC_COMPILE_OBJECT
	"<CMAKE_RC_COMPILER> --output-format=coff <FLAGS> <DEFINES> -o <OBJECT> <SOURCE>")

# Append the CFLAGS and LDFLAGS.
SET(GENS_C_FLAGS_COMMON "${GENS_C_FLAGS_COMMON} ${GENS_C_FLAGS_WIN32}")
SET(GENS_CXX_FLAGS_COMMON "${GENS_CXX_FLAGS_COMMON} ${GENS_C_FLAGS_WIN32}")
SET(GENS_EXE_LINKER_FLAGS_COMMON "${GENS_EXE_LINKER_FLAGS_COMMON} ${GENS_EXE_LINKER_FLAGS_WIN32}")
SET(GENS_SHARED_LINKER_FLAGS_COMMON "${GENS_SHARED_LINKER_FLAGS_COMMON} ${GENS_EXE_LINKER_FLAGS_WIN32} ${GENS_SHARED_LINKER_FLAGS_WIN32}")
UNSET(GENS_C_FLAGS_WIN32)
UNSET(GENS_EXE_LINKER_FLAGS_WIN32)
