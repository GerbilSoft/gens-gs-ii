PROJECT(gens-qt4)
cmake_minimum_required(VERSION 2.6)

# Find Qt4.
SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt4)
INCLUDE(${QT_USE_FILE})

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES(${gens-gs-ii_BINARY_DIR})

# Include the previous directory.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../)

# Compatibility sources.
IF(WIN32)
	SET(gens-qt4_COMPAT_SRC lg_win32.c)
ELSE()
	SET(gens-qt4_COMPAT_SRC)
ENDIF()

# Sources.
SET(gens-qt4_SRCS
	gqt4_main.cpp
	GensWindow.cpp
	AboutWindow.cpp
	GensMenuBar.cpp
	GensQGLWidget.cpp
	)

# Headers with Qt4 objects.
SET(gens-qt4_MOC_HEADERS
	GensWindow.hpp
	AboutWindow.hpp
	GensMenuBar.hpp
	GensQGLWidget.hpp
	)

# Create .moc files for sources that need them.
QT4_WRAP_CPP(gens-qt4_MOC_SOURCES ${gens-qt4_MOC_HEADERS})

# UI files.
SET(gens-qt4_UIS
	ui/AboutWindow.ui
	)

# Generate the header files from the UI files.
QT4_WRAP_UI(gens-qt4_UIS_H ${gens-qt4_UIS})

# Include the binary directory. (Required for the .h files.)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# Resource files.
SET(gens-qt4_RCC_SRCS
	ui/resources/gens-qt4.qrc
	)

# Add the resource files to the project.
QT4_ADD_RESOURCES(gens-qt4_RCC_O ${gens-qt4_RCC_SRCS})

# Compatibility sources.
IF(WIN32)
	SET(gens-qt4_COMPAT_SRCS gqt4_win32.cpp)
ELSE()
	SET(gens-qt4_COMPAT_SRCS)
ENDIF()

# Build the executable.
ADD_EXECUTABLE(gens-qt4 ${gens-qt4_SRCS} ${gens-qt4_COMPAT_SRCS} ${gens-qt4_MOC_SOURCES} ${gens-qt4_UIS_H} ${gens-qt4_RCC_O})
TARGET_LINK_LIBRARIES(gens-qt4 gens ${QT_LIBRARIES})
ADD_DEPENDENCIES(gens-qt4 gens)
