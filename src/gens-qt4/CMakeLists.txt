PROJECT(gens-qt4)
cmake_minimum_required(VERSION 2.6)

# Find Qt4.
FIND_PACKAGE(Qt4)
INCLUDE(${QT_USE_FILE})

# Include the previous directory.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../)

# Sources.
SET(gens-qt4_SRCS
	gqt4_main.cpp
	GensWindow.cpp
	)

# Headers.
SET(gens-qt4_HEADERS
	gqt4_main.hpp
	SdlWidget.hpp
	GensWindow.hpp
	)

# Create .moc files for sources that need them.
QT4_WRAP_CPP(gens-qt4_MOC_SOURCES ${gens-qt4_HEADERS})

# UI files.
SET(gens-qt4_UIS
	ui/GensWindow.ui
	)

# Generate the header files from the UI files.
QT4_WRAP_UI(gens-qt4_UIS_H ${gens-qt4_UIS})

# Include the binary directory. (Required for the .h files.)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# Resource files.
SET(gens-qt4_RCC_SRCS)

# Add the resource files to the project.
QT4_ADD_RESOURCES(gens-qt4_RCC_O ${gens-qt4_RCC_SRCS})

# Build the executable.
ADD_EXECUTABLE(gens-qt4 ${gens-qt4_SRCS} ${gens-qt4_MOC_SOURCES} ${gens-qt4_UIS_H} ${gens-qt4_RCC_O})
TARGET_LINK_LIBRARIES(gens-qt4 gens ${QT_LIBRARIES})
ADD_DEPENDENCIES(gens-qt4 gens)
