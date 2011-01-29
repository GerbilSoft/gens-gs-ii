PROJECT(gens-qt4)
cmake_minimum_required(VERSION 2.6)

# Find Qt4.
# TODO: Determine the actual minimum version.
# 4.1.0: QCoreApplication::arguments()
FIND_PACKAGE(Qt4 4.1.0 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
INCLUDE(${QT_USE_FILE})

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES("${gens-gs-ii_BINARY_DIR}")

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")

# LibGens source and binary directories.
# Binary directory is needed for config.h.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# Library checks.
INCLUDE(CheckLibraryExists)
CHECK_LIBRARY_EXISTS(c sigaction "" HAVE_SIGACTION)	# libc: sigaction()

# Find OpenGL.
# TODO: Disable OpenGL functionality if it isn't found.
FIND_PACKAGE(OpenGL REQUIRED)
IF(OPENGL_LIBRARIES)
	# Found OpenGL.
	SET(HAVE_OPENGL 1)
	MESSAGE(STATUS "Found OpenGL: ${OPENGL_gl_LIBRARY}")
	INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
	
	# Find GLEW.
	SET(OLD_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
	SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${OPENGL_gl_LIBRARY})
	IF(WIN32)
		SET(CHECK_GLEW_LIBRARY "glew32")
	ELSE()
		SET(CHECK_GLEW_LIBRARY "GLEW")
	ENDIF()
	CHECK_LIBRARY_EXISTS(${CHECK_GLEW_LIBRARY} glewInit "" HAVE_GLEW)
	IF(HAVE_GLEW)
		SET(GLEW_LIBRARY "-l${CHECK_GLEW_LIBRARY}")
	ENDIF(HAVE_GLEW)
	UNSET(CHECK_GLEW_LIBRARY)
	SET(CMAKE_REQUIRED_LIBRARIES ${OLD_CMAKE_REQUIRED_LIBRARIES})
	UNSET(OLD_CMAKE_REQUIRED_LIBRARIES)
ENDIF()

# Find PortAudio.
# TODO: Write a CMake file for PortAudio.
CHECK_LIBRARY_EXISTS(portaudio Pa_Initialize "" HAVE_PORTAUDIO)	# libportaudio: Pa_Initialize()
IF(HAVE_PORTAUDIO)
	# Found PortAudio.
	# TODO: Check Win32 and Mac OS X library naming.
	SET(PORTAUDIO_LIBRARY "portaudio")
	MESSAGE(STATUS "Found PortAudio: ${PORTAUDIO_LIBRARY}")
ENDIF()

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h")

# Sources.
SET(gens-qt4_SRCS
	gqt4_main.cpp
	GensWindow.cpp
	AboutWindow.cpp
	GensMenuBar.cpp
	EmuThread.cpp
	GensQApplication.cpp
	SigHandler.cpp
	VBackend/VBackend.cpp
	VBackend/GensQGLWidget.cpp
	Input/KeyHandlerQt.cpp
	CtrlConfigWindow.cpp
	Audio/GensPortAudio.cpp
	EmuManager.cpp
	Audio/ABackend.cpp
	VBackend/GLShaderManager.cpp
	Audio/ARingBuffer.cpp
	ZipSelectDialog.cpp
	Input/EventKeys.cpp
	EmuManager_qEmu.cpp
	VBackend/GlTex2D.cpp
	GeneralConfigWindow.cpp
	widgets/GensLineEdit.cpp
	GensConfig.cpp
	)

# Headers with Qt4 objects.
SET(gens-qt4_MOC_HEADERS
	GensWindow.hpp
	AboutWindow.hpp
	GensMenuBar.hpp
	EmuThread.hpp
	GensQApplication.hpp
	VBackend/GensQGLWidget.hpp
	CtrlConfigWindow.hpp
	EmuManager.hpp
	VBackend/MsgTimer.hpp
	ZipSelectDialog.hpp
	Input/EventKeys.hpp
	GeneralConfigWindow.hpp
	widgets/GensLineEdit.hpp
	GensConfig.hpp
	)

# Create .moc files for sources that need them.
QT4_WRAP_CPP(gens-qt4_MOC_SOURCES ${gens-qt4_MOC_HEADERS})

# UI files.
SET(gens-qt4_UIS
	ui/AboutWindow.ui
	ui/CtrlConfigWindow.ui
	ui/ZipSelectDialog.ui
	ui/GeneralConfigWindow.ui
	)

# Generate the header files from the UI files.
QT4_WRAP_UI(gens-qt4_UIS_H ${gens-qt4_UIS})

# Resource files.
SET(gens-qt4_RCC_SRCS
	ui/resources/gens-qt4.qrc
	)

# Add the resource files to the project.
QT4_ADD_RESOURCES(gens-qt4_RCC_O ${gens-qt4_RCC_SRCS})

# Compatibility sources.
IF(WIN32)
	SET(gens-qt4_COMPAT_SRCS gqt4_win32.cpp ui/gens-qt4.rc)
ELSE()
	SET(gens-qt4_COMPAT_SRCS)
ENDIF()

# Build the executable.
ADD_EXECUTABLE(gens-qt4 ${gens-qt4_SRCS} ${gens-qt4_COMPAT_SRCS} ${gens-qt4_MOC_SOURCES} ${gens-qt4_UIS_H} ${gens-qt4_RCC_O})
TARGET_LINK_LIBRARIES(gens-qt4 gens)
IF(WIN32)
	TARGET_LINK_LIBRARIES(gens-qt4)
ENDIF(WIN32)
TARGET_LINK_LIBRARIES(gens-qt4 ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTOPENGL_LIBRARY})
IF(HAVE_GLEW)
	TARGET_LINK_LIBRARIES(gens-qt4 ${GLEW_LIBRARY})
ENDIF(HAVE_GLEW)
TARGET_LINK_LIBRARIES(gens-qt4 ${OPENGL_gl_LIBRARY})
IF(HAVE_PORTAUDIO)
	TARGET_LINK_LIBRARIES(gens-qt4 ${PORTAUDIO_LIBRARY})
ENDIF(HAVE_PORTAUDIO)

IF(WIN32)
	# Use the static version of GLEW on Win32.
	# TODO: Make sure that the DLL version isn't used by accident!
	IF(HAVE_GLEW)
		SET_TARGET_PROPERTIES(gens-qt4
			PROPERTIES COMPILE_FLAGS -DGLEW_STATIC)
	ENDIF(HAVE_GLEW)
ENDIF(WIN32)

ADD_DEPENDENCIES(gens-qt4 gens)
