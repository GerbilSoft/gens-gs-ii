# Win32-specific CFLAGS/CXXFLAGS.
# For Microsoft Visual C++ compilers.

# Set minimum Windows version to Windows 2000. (Windows NT 5.0)
SET(GENS_CFLAG_WIN32_WINNT "-D_WIN32_WINNT=0x0500")

# Release build: Prefer static libraries.
IF(CMAKE_BUILD_TYPE MATCHES ^release)
	SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ENDIF(CMAKE_BUILD_TYPE MATCHES ^release)

# Initialize CFLAGS/CXXFLAGS.
SET(GENS_CFLAGS_PLATFORM "-Zc:wchar_t ${GENS_CFLAG_WIN32_WINNT}")
SET(GENS_CXXFLAGS_PLATFORM "-Zc:wchar_t ${GENS_CFLAG_WIN32_WINNT}")

# Unset variables we don't need elsewhere.
UNSET(GENS_CFLAG_WIN32_WINNT)
