PROJECT(libgenscd)
cmake_minimum_required(VERSION 2.6.0)

# Sources.
SET(libgenscd_SRCS
	CdDrive.cpp
	ScsiBase.cpp
	scsi_errors.c
	)

IF(WIN32)
	SET(libgenscd_SRCS
		${libgenscd_SRCS}
		ScsiSpti.cpp
		)
ENDIF(WIN32)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	SET(libgenscd_SRCS
		${libgenscd_SRCS}
		ScsiLinux.cpp
		)
ENDIF()

######################
# Build the library. #
######################

SET(LIBGENSCD_PDB_FILE "${CMAKE_CURRENT_BINARY_DIR}/genscd.pdb")

IF(MSVC)
	# CMake seems to use weird settings for the PDB file.
	# (at least version 2.8.12.2; 3.0.0 might be different)
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_C}"
		"/Fd${LIBGENSCD_PDB_FILE}"
		CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT}")
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_CXX}"
		"/Fd${LIBGENSCD_PDB_FILE}"
		CMAKE_CXX_COMPILE_OBJECT "${CMAKE_CXX_COMPILE_OBJECT}")
ENDIF(MSVC)

ADD_LIBRARY(genscd STATIC ${libgenscd_SRCS})
TARGET_LINK_LIBRARIES(genscd genstext)

# LibGensCD test program.
SET(genscd_test_SRCS
	CdDriveTest.cpp
	)

# Build the test program.
ADD_EXECUTABLE(genscd_test ${genscd_test_SRCS})
TARGET_LINK_LIBRARIES(genscd_test genscd)
