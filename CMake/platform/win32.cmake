# Win32-specific CFLAGS/CXXFLAGS.
SET(GENS_CFLAGS_PLATFORM "-fshort-wchar")
SET(GENS_CXXFLAGS_PLATFORM "-fshort-wchar")

# Debug flags.
IF(CMAKE_BUILD_TYPE MATCHES ^debug)
	SET(GENS_CFLAGS_PLATFORM "-gstabs")
	SET(GENS_CXXFLAGS_PLATFORM "-gstabs")
ENDIF(CMAKE_BUILD_TYPE MATCHES ^debug)

# Test for static libgcc/libstdc++.
# NOTE: libstdc++ check removed for now.
SET(GENS_LDFLAGS_PLATFORM "")
FOREACH(FLAG_TEST "-static-libgcc" "-Wl,--large-address-aware" "-Wl,--nxcompat" "-Wl,--dynamicbase") # "-static-libstdc++")
	CHECK_C_COMPILER_FLAG("${FLAG_TEST}" LDFLAG_${FLAG_TEST})
	IF(LDFLAG_${FLAG_TEST})
		SET(GENS_LDFLAGS_PLATFORM "${GENS_LDFLAGS_PLATFORM} ${FLAG_TEST}")
	ENDIF(LDFLAG_${FLAG_TEST})
	UNSET(LDFLAG_${FLAG_TEST})
ENDFOREACH()

# Enable windres support on MinGW.
# http://www.cmake.org/Bug/view.php?id=4068
IF(MINGW)
	SET(CMAKE_RC_COMPILER_INIT windres)
	ENABLE_LANGUAGE(RC)
	
	# NOTE: Setting CMAKE_RC_OUTPUT_EXTENSION doesn't seem to work.
	# Force windres to output COFF, even though it'll use the .res extension.
	SET(CMAKE_RC_OUTPUT_EXTENSION .obj)
	SET(CMAKE_RC_COMPILE_OBJECT
		"<CMAKE_RC_COMPILER> --output-format=coff <FLAGS> <DEFINES> -o <OBJECT> <SOURCE>")
ENDIF(MINGW)
