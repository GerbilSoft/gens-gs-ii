# Check for LZMA SDK.
# TODO: Maybe add support for external LZMA SDK.
# For now, use the internal LZMA SDK.
IF(USE_LZMA)
	# LZMA base library.
	SET(LZMA_LIBRARY lzmabase)
	SET(LZMA_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/extlib/lzma/lzmabase")
	SET(HAVE_LZMA 1)
	SET(USE_INTERNAL_LZMA 1)

	# 7z library.
	SET(7z_LIBRARY 7z)
	SET(7z_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/extlib/lzma/7z")
	SET(HAVE_7z 1)
	SET(USE_INTERNAL_7z 1)
ENDIF(USE_LZMA)
