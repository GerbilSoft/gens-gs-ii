PROJECT(gens-qt4)
cmake_minimum_required(VERSION 2.6)

# Find Qt4.
# TODO: Determine the actual minimum version.
# 4.1.0: QCoreApplication::arguments()
# 4.2.0: QString::fromWCharArray() [Win32]
# 4.2.0: QMainWindow::menuWidget()
# 4.3.0: QMainWindow::setUnifiedTitleAndToolBarOnMac() [Mac OS X]
# 4.6.0: QPixmap::fromWinHICON() [Win32] (fallbacks to QPixmap::fromWinHBITMAP() are in place)
IF(WIN32)
	SET(GENS_QT4_MIN_VERSION 4.2.0)
	SET(GENS_QT4_COMPONENTS QtCore QtGui QtOpenGL)
ELSEIF(APPLE)
	SET(GENS_QT4_MIN_VERSION 4.3.0)
	SET(GENS_QT4_COMPONENTS QtCore QtGui QtOpenGL)
ELSE()
	SET(GENS_QT4_MIN_VERSION 4.2.0)
	SET(GENS_QT4_COMPONENTS QtCore QtGui QtOpenGL QtDBus)
ENDIF()

# Find Qt4.
# Components used:
# - QtCore
# - QtGui
# - QtOpenGl
# - QtDBus (Unix only, and not Mac OS X)
FIND_PACKAGE(Qt4 ${GENS_QT4_MIN_VERSION}
	COMPONENTS ${GENS_QT4_COMPONENTS})
IF(NOT QT_QTCORE_FOUND)
	MESSAGE(FATAL_ERROR "QtCore not found! Please install Qt ${GENS_QT4_MIN_VERSION} or higher.")
ELSEIF(NOT QT_QTGUI_FOUND)
	MESSAGE(FATAL_ERROR "QtGui not found! Please install Qt ${GENS_QT4_MIN_VERSION} or higher.")
ELSEIF(NOT QT_QTOPENGL_FOUND)
	MESSAGE(FATAL_ERROR "QtOpenGL not found! Please install Qt ${GENS_QT4_MIN_VERSION} or higher.")
ENDIF()

IF(APPLE)
	# Don't enable QtDBus on Mac OS X.
	UNSET(QT_QTDBUS_FOUND)
ENDIF(APPLE)

INCLUDE(${QT_USE_FILE})

IF(WIN32 AND QT_IS_STATIC)
	# Win32: Qt Static requires some extra libraries.
	SET(WIN32_LIBS ws2_32 winmm imm32)
ENDIF(WIN32 AND QT_IS_STATIC)

# Link to Carbon on Mac OS X.
# TODO: Use Cocoa instead if we're compiling against Qt/Cocoa.
IF(APPLE)
	#SET(GUI_TYPE MACOSX_BUNDLE)
	IF(QT_MAC_USE_COCOA)
		# Qt is using Cocoa.
		FIND_LIBRARY(FOUNDATION_LIBRARY Foundation)
		MARK_AS_ADVANCED(FOUNDATION_LIBRARY)
		SET(APPLE_LIBS ${FOUNDATION_LIBRARY})
	ELSE(QT_MAC_USE_COCOA)
		# Qt is using Carbon.
		INCLUDE_DIRECTORIES ( /Developer/Headers/FlatCarbon )
		FIND_LIBRARY(CARBON_LIBRARY Carbon)
		MARK_AS_ADVANCED(CARBON_LIBRARY)
		SET(APPLE_LIBS ${CARBON_LIBRARY})
	ENDIF(QT_MAC_USE_COCOA)
ENDIF(APPLE)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES("${gens-gs-ii_BINARY_DIR}")

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../")

# GensQt4 source and binary directories.
# Binary directory is needed for config.h.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# ZLIB include directory.
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

# Include the OpenGL directory.
# TODO: FIND_PACKAGE(OpenGL) here even though it's in the root CMakeLists.txt?
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
IF(GLEW_INCLUDE_DIR)
	INCLUDE_DIRECTORIES(${GLEW_INCLUDE_DIR})
ENDIF(GLEW_INCLUDE_DIR)

# Library checks.
INCLUDE(CheckLibraryExists)
CHECK_LIBRARY_EXISTS(c sigaction "" HAVE_SIGACTION)	# libc: sigaction()

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
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.gens-qt4.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.gens-qt4.h")

# Sources.
SET(gens-qt4_COMMON_SRCS
	gqt4_main.cpp
	EmuThread.cpp
	IdleThread.cpp
	GensQApplication.cpp
	SigHandler.cpp
	VBackend/VBackend.cpp
	VBackend/VBackend_OSD.cpp
	VBackend/FpsManager.cpp
	VBackend/GensQGLWidget.cpp
	VBackend/GLBackend.cpp
	Input/KeyHandlerQt.cpp
	Audio/GensPortAudio.cpp
	EmuManager.cpp
	Audio/ABackend.cpp
	VBackend/GLShaderManager.cpp
	VBackend/GLShader.cpp
	VBackend/GLShaderPaused.cpp
	VBackend/GLShaderFastBlur.cpp
	Audio/ARingBuffer.cpp
	EmuManager_qEmu.cpp
	VBackend/GLTex2D.cpp
	EmuManager_str.cpp
	cdrom/FindCdromDrives.cpp
	cdrom/FindCdromBase.cpp
	)

SET(gens-qt4_WINDOW_SRCS
	windows/GensWindow.cpp
	windows/GensWindow_menu_slots.cpp
	windows/GensWindow_menu_init.cpp
	windows/GensWindow_menu_sync.cpp
	windows/GensMenuShortcuts.cpp
	windows/AboutDialog.cpp
	windows/CtrlConfigWindow.cpp
	windows/ZipSelectDialog.cpp
	windows/GeneralConfigWindow.cpp
	windows/GeneralConfigWindow_p.cpp
	windows/GeneralConfigWindow_slots.cpp
	windows/McdControlWindow.cpp
	)

SET(gens-qt4_WINDOW_H
	windows/GensWindow_p.hpp
	windows/GensMenuShortcuts_p.hpp
	windows/GeneralConfigWindow_p.hpp
	)

SET(gens-qt4_WIDGET_SRCS
	widgets/GensLineEdit.cpp
	widgets/GensZipDirModel.cpp
	widgets/GensZipDirItem.cpp
	widgets/GensColorComboBox.cpp
	widgets/GensKeySequenceWidget.cpp
	widgets/GensCtrlCfgWidget.cpp
	widgets/GensCtrlKeyWidget.cpp
	widgets/GensColorButton.cpp
	widgets/KeyNames.cpp
	widgets/RecentRomsMenu.cpp
	)

SET(gens-qt4_CONFIG_SRCS
	Config/ConfigStore.cpp
	Config/ConfigDefaults.cpp
	Config/PathConfig.cpp
	Config/CtrlConfig.cpp
	Config/RecentRoms.cpp
	ConfigHandler.cpp
	)

SET(gens-qt4_ROMDB_SRCS
	RomDB/McdRomDB.cpp
	)

# Headers with Qt4 objects.
SET(gens-qt4_COMMON_MOC_HEADERS
	EmuThread.hpp
	IdleThread.hpp
	GensQApplication.hpp
	VBackend/VBackend.hpp
	VBackend/FpsManager.hpp
	VBackend/GLBackend.hpp
	VBackend/GensQGLWidget.hpp
	VBackend/GensQGLWidget_p.hpp
	EmuManager.hpp
	Input/KeyHandlerQt.hpp
	cdrom/FindCdromDrives.hpp
	cdrom/FindCdromBase.hpp
	)

SET(gens-qt4_WINDOW_MOC_HEADERS
	windows/GensWindow.hpp
	windows/GensMenuShortcuts.hpp
	windows/AboutDialog.hpp
	windows/CtrlConfigWindow.hpp
	windows/ZipSelectDialog.hpp
	windows/GeneralConfigWindow.hpp
	windows/McdControlWindow.hpp
	)

SET(gens-qt4_WIDGET_MOC_HEADERS
	widgets/GensLineEdit.hpp
	widgets/GensZipDirModel.hpp
	widgets/GensColorComboBox.hpp
	widgets/GensKeySequenceWidget.hpp
	widgets/GensKeySequenceWidget_p.hpp
	widgets/GensCtrlCfgWidget.hpp
	widgets/GensCtrlKeyWidget.hpp
	widgets/GensCtrlKeyWidget_p.hpp
	widgets/GensColorButton.hpp
	widgets/RecentRomsMenu.hpp
	)

SET(gens-qt4_CONFIG_MOC_HEADERS
	Config/ConfigStore.hpp
	Config/PathConfig.hpp
	Config/RecentRoms.hpp
	ConfigHandler.hpp
	)

SET(gens-qt4_ROMDB_MOC_HEADERS
	RomDB/McdRomDB.hpp
	)

# UI files.
SET(gens-qt4_WINDOW_UIS
	windows/GensWindow.ui
	windows/AboutDialog.ui
	windows/CtrlConfigWindow.ui
	windows/ZipSelectDialog.ui
	windows/GeneralConfigWindow.ui
	windows/McdControlWindow.ui
	)

# Generate the header files from the UI files.
QT4_WRAP_UI(gens-qt4_UIS_H ${gens-qt4_WINDOW_UIS})

#########################################
# System-specific CD-ROM drive support. #
#########################################

IF(QT_QTDBUS_FOUND)
	# QtDBus found.
	# Enable UDisks2 support.
	# NOTE: The D-BUS interfaces are generated in ${CMAKE_CURRENT_BINARY_DIR},
	# *not* ${CMAKE_CURRENT_BINARY_DIR}/cdrom/dbus/.
	INCLUDE(QtDBusInterfaceNoNS)
	QT4_ADD_DBUS_INTERFACES_NONS(
		gens-qt4-UDISKS2_DBUS_SRCS
		INCLUDE cdrom/dbus/DBusMetatypes.hpp
		cdrom/dbus/org.freedesktop.UDisks2.xml
		cdrom/dbus/org.freedesktop.DBus.ObjectManager.xml
		cdrom/dbus/org.freedesktop.DBus.Properties.xml
		)
	QT4_ADD_DBUS_INTERFACES_NONS(
		gens-qt4-UDISKS_DBUS_SRCS
		INCLUDE cdrom/dbus/DBusMetatypes.hpp
		cdrom/dbus/org.freedesktop.UDisks.xml
		cdrom/dbus/org.freedesktop.UDisks.Device.xml
		)
	
	SET(gens-qt4_DBUS_SRCS
		cdrom/FindCdromUDisks2.cpp
		cdrom/FindCdromUDisks.cpp
		${gens-qt4-UDISKS2_DBUS_SRCS}
		${gens-qt4-UDISKS_DBUS_SRCS}
		)
	SET(gens-qt4_DBUS_MOC_HEADERS
		cdrom/FindCdromUDisks2.hpp
		cdrom/FindCdromUDisks.hpp
		)
ENDIF(QT_QTDBUS_FOUND)

IF(UNIX AND NOT APPLE)
	SET(gens-qt4_PLATFORM_SRCS
		cdrom/FindCdromUnix.cpp
		)
	SET(gens-qt4_PLATFORM_MOC_HEADERS
		cdrom/FindCdromUnix.hpp
		)
ELSEIF(WIN32)
	# Win32: Enable Win32 CD-ROM suport.
	SET(gens-qt4_PLATFORM_SRCS
		cdrom/FindCdromWin32.cpp
		)
	SET(gens-qt4_PLATFORM_MOC_HEADERS
		cdrom/FindCdromWin32.hpp
		)
ENDIF()

############################
# System-specific UI code. #
############################

IF(WIN32)
	SET(gens-qt4_PLATFORM_UI_SRCS
		GensQApplication_win32.cpp
		resources/win32/gens-qt4.rc
		)
ELSEIF(APPLE)
	SET(gens-qt4_PLATFORM_UI_SRCS
		window/GeneralConfigWindow_mac.mm
		resources/mac/gensgs.icns
		)
ENDIF()

# Create .moc files for sources that need them.
QT4_WRAP_CPP(gens-qt4_MOC_SOURCES
		${gens-qt4_COMMON_MOC_HEADERS}
		${gens-qt4_WINDOW_MOC_HEADERS}
		${gens-qt4_WIDGET_MOC_HEADERS}
		${gens-qt4_CONFIG_MOC_HEADERS}
		${gens-qt4_ROMDB_MOC_HEADERS}
		${gens-qt4_DBUS_MOC_HEADERS}
		${gens-qt4_PLATFORM_MOC_HEADERS}
		)

# Always include the Oxygen icons on Win32 and Mac OS X.
IF(WIN32 OR APPLE)
	SET(GENS_INCLUDE_OXYGEN_ICONS 1)
ENDIF()

# Resource files.
SET(gens-qt4_RCC_SRCS
	resources/gens-qt4.qrc
	)
IF(GENS_INCLUDE_OXYGEN_ICONS)
	SET(gens-qt4_RCC_SRCS
		${gens-qt4_RCC_SRCS}
		resources/oxygen/oxygen.qrc
		)
ENDIF(GENS_INCLUDE_OXYGEN_ICONS)

# Add the resource files to the project.
QT4_ADD_RESOURCES(
	gens-qt4_RCC_O ${gens-qt4_RCC_SRCS}
	OPTIONS -no-compress
	)

# Mac OS X bundle.
IF(APPLE)
	SET_SOURCE_FILES_PROPERTIES(${gens-qt4_MAC_ICONS}
		PROPERTIES MACOSX_PACKAGE_LOCATION Resources
		)
ENDIF(APPLE)

#################
# Translations. #
#################

ADD_SUBDIRECTORY(translations)
# TODO: Mac OS X bundle location.
# TODO: "make install" location.

#########################
# Build the executable. #
#########################

ADD_EXECUTABLE(gens-qt4
	WIN32
	${gens-qt4_SRCS}
	${gens-qt4_COMMON_SRCS} ${gens-qt4_COMMON_MOC_HEADERS}
	${gens-qt4_WINDOW_SRCS} ${gens-qt4_WINDOW_H} ${gens-qt4_WINDOW_MOC_HEADERS}
	${gens-qt4_WIDGET_SRCS} ${gens-qt4_WIDGET_MOC_HEADERS}
	${gens-qt4_CONFIG_SRCS} ${gens-qt4_CONFIG_MOC_HEADERS}
	${gens-qt4_ROMDB_SRCS} ${gens-qt4_ROMDB_MOC_HEADERS}
	${gens-qt4_DBUS_SRCS} ${gens-qt4_DBUS_MOC_HEADERS}
	${gens-qt4_PLATFORM_SRCS} ${gens-qt4_PLATFORM_MOC_HEADERS}
	${gens-qt4_PLATFORM_UI_SRCS}
	${gens-qt4_MOC_SOURCES}
	${gens-qt4_UIS_H}
	${gens-qt4_RCC_O}
	)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(gens-qt4)
TARGET_LINK_LIBRARIES(gens-qt4 gens ${ZLIB_LIBRARY} genscd genskeys)
ADD_DEPENDENCIES(gens-qt4 translations_target)

# Additional libraries.
IF(APPLE AND APPLE_LIBS)
	TARGET_LINK_LIBRARIES(gens-qt4 ${APPLE_LIBS})
ENDIF(APPLE AND APPLE_LIBS)
TARGET_LINK_LIBRARIES(gens-qt4 ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTOPENGL_LIBRARY})
IF(QT_QTDBUS_FOUND)
	TARGET_LINK_LIBRARIES(gens-qt4 ${QT_QTDBUS_LIBRARY})
ENDIF(QT_QTDBUS_FOUND)
IF(HAVE_GLEW)
	TARGET_LINK_LIBRARIES(gens-qt4 ${GLEW_LIBRARY})
	ADD_DEFINITIONS(${GLEW_COMPILE_DEFINITIONS})
ENDIF(HAVE_GLEW)
TARGET_LINK_LIBRARIES(gens-qt4 ${OPENGL_gl_LIBRARY})
IF(HAVE_PORTAUDIO)
	TARGET_LINK_LIBRARIES(gens-qt4 ${PORTAUDIO_LIBRARY})
ENDIF(HAVE_PORTAUDIO)

# Split debug information.
IF(SPLIT_DEBUG)
	INCLUDE(SplitDebugInformation)
	SPLIT_DEBUG_INFORMATION(gens-qt4)
ENDIF(SPLIT_DEBUG)

# Compress the EXE.
IF(COMPRESS_EXE)
	INCLUDE(CompressExeWithUpx)
	COMPRESS_EXE_WITH_UPX(gens-qt4)
ENDIF(COMPRESS_EXE)

# Qt options:
# - Fast QString concatenation. (Qt 4.6+, plus 4.8-specific version)
# - Disable implicit QString ASCII casts.
ADD_DEFINITIONS(-DQT_USE_FAST_CONCATENATION
	-DQT_USE_FAST_OPERATOR_PLUS
	-DQT_USE_QSTRINGBUILDER
	-DQT_NO_CAST_FROM_ASCII
	-DQT_NO_CAST_TO_ASCII
	)

# ZLIB definitions.
ADD_DEFINITIONS(${ZLIB_DEFINITIONS})

# Mac OS X bundle information.
# TODO: Add "NO-EMU" if necessary.
SET(MACOSX_BUNDLE_GUI_IDENTIFIER "gsft.gerbilsoft.GensGSII")
SET(MACOSX_BUNDLE_BUNDLE_NAME "Gens/GS II")
SET(MACOSX_BUNDLE_ICON_FILE "gensgs.icns")
SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "0.0.0")
SET(MACOSX_BUNDLE_LONG_VERSION_STRING "0.0.0 (Development Build)")
IF(NOT GENS_ENABLE_EMULATION)
	SET(MACOSX_BUNDLE_LONG_VERSION_STRING "${MACOSX_BUNDLE_LONG_VERSION_STRING} (NO-EMU)")
ENDIF(NOT GENS_ENABLE_EMULATION)

# Mac OS X: Set a custom info.plist file for the application bundle.
SET_PROPERTY(TARGET gens-qt4
	PROPERTY MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/mac/Info-CMake.plist")
