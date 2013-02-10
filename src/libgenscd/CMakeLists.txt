PROJECT(libgenscd)
cmake_minimum_required(VERSION 2.6.0)

# Sources.
SET(libgenscd_SRCS
	CdDrive.cpp
	)

IF(WIN32)
	SET(libgenscd_SRCS
		${libgenscd_SRCS}
		CdDriveSpti.cpp
		)
ENDIF(WIN32)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	SET(libgenscd_SRCS
		${libgenscd_SRCS}
		CdDriveLinux.cpp
		)
ENDIF()

# Build the library.
ADD_LIBRARY(genscd STATIC ${libgenscd_SRCS})

# LibGensCD test program.
SET(genscd_test_SRCS
	CdDriveTest.cpp
	)

# Build the test program.
ADD_EXECUTABLE(genscd_test ${genscd_test_SRCS})
TARGET_LINK_LIBRARIES(genscd_test genscd)
