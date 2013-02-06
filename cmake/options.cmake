# Build options.

# LZMA SDK.
OPTION(USE_LZMA			"Use the LZMA SDK to decompress 7-Zip archives." 		1)

# ZLIB.
# TODO: Make ZLIB optional? (This would disable savestates...)
IF(NOT WIN32)
	OPTION(USE_INTERNAL_ZLIB	"Always use the internal copy of ZLIB." 		0)
ELSE(NOT WIN32)
	# TODO: Allow use of external zlib on Win32?
	SET(USE_INTERNAL_ZLIB 1)
ENDIF(NOT WIN32)

# OpenGL.
# TODO: Make OpenGL optional once other backends have been added.
OPTION(USE_GLEW			"Use GLEW for OpenGL extension support." 			1)
IF(NOT WIN32)
	OPTION(USE_INTERNAL_GLEW	"Always use the internal copy of GLEW." 		0)
ELSE(NOT WIN32)
	# Always use the internal GLEW on Win32 regardless.
	# TODO: Allow use of external GLEW on Win32?
	SET(USE_INTERNAL_GLEW 1)
ENDIF(NOT WIN32)

# Additional stuff.
OPTION(BUILD_DOC		"Build documentation." 						1)
