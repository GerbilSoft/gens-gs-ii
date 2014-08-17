SET(HOST_SYSTEM i686-w64-mingw32)

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_SYSTEM_PROCESSOR i686)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER	${HOST_SYSTEM}-gcc)
SET(CMAKE_CXX_COMPILER	${HOST_SYSTEM}-g++)
SET(CMAKE_RC_COMPILER	${HOST_SYSTEM}-windres)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH /usr/${HOST_SYSTEM})

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# FindQt4.cmake querys qmake to get information, this doesn't work when crosscompiling
SET(QT_BINARY_DIR		$ENV{QTDIR}/bin)
SET(QT_HEADERS_DIR		$ENV{QTDIR}/include)
SET(QT_INCLUDE_DIR		$ENV{QTDIR}/include)
SET(QT_LIBRARY_DIR		$ENV{QTDIR}/bin)

SET(QT_QTCORE_INCLUDE_DIR	${QT_INCLUDE_DIR}/QtCore)
SET(QT_QTGUI_INCLUDE_DIR	${QT_INCLUDE_DIR}/QtGui)
SET(QT_QTOPENGL_INCLUDE_DIR	${QT_INCLUDE_DIR}/QtOpenGL)

SET(QT_QMAKE_EXECUTABLE		${QT_BINARY_DIR}/qmake)
