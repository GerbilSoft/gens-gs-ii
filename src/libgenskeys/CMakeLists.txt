PROJECT(libgenskeys)
cmake_minimum_required(VERSION 2.6.0)

# Sources.
SET(libgenskeys_SRCS
	KeyManager.cpp
	)
SET(libgenskeys_H
	KeyManager.hpp
	)

# Build the library.
ADD_LIBRARY(genskeys STATIC
	${libgenskeys_SRCS}
	${libgenskeys_H}
	)
TARGET_LINK_LIBRARIES(genskeys gens)
