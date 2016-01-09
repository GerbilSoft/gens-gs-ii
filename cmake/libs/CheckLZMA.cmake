# Check for LZMA SDK.
# TODO: Maybe add support for external LZMA SDK.
# For now, use the internal LZMA SDK.
IF(USE_LZMA)
	# LZMA SDK. Includes 7z archive support.
	SET(LZMA_LIBRARY lzma)
	SET(LZMA_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/extlib/lzma")
	SET(HAVE_LZMA 1)
	SET(USE_INTERNAL_LZMA 1)
ENDIF(USE_LZMA)
