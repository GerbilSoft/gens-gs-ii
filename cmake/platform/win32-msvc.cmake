# Win32-specific CFLAGS/CXXFLAGS.
# For Microsoft Visual C++ compilers.

# Basic platform flags:
# - wchar_t should be a distinct type.
# - Set minimum Windows version to Windows 2000. (Windows NT 5.0)
# - Enable strict type checking in the Windows headers.
# - Define WIN32_LEAN_AND_MEAN to reduce the number of Windows headers included.
# - Define NOMINMAX to disable the MIN() and MAX() macros.
SET(GENS_C_FLAGS_WIN32 "-Zc:wchar_t -D_WIN32_WINNT=0x0500 -DSTRICT -DWIN32_LEAN_AND_MEAN -DNOMINMAX")

# Release build: Prefer static libraries.
IF(CMAKE_BUILD_TYPE MATCHES ^release)
	SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ENDIF(CMAKE_BUILD_TYPE MATCHES ^release)

# Append the CFLAGS.
SET(GENS_C_FLAGS_COMMON "${GENS_C_FLAGS_COMMON} ${GENS_C_FLAGS_WIN32}")
SET(GENS_CXX_FLAGS_COMMON "${GENS_CXX_FLAGS_COMMON} ${GENS_C_FLAGS_WIN32}")
UNSET(GENS_C_FLAGS_WIN32)
