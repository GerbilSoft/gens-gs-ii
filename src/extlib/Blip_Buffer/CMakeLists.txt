PROJECT(Blip_Buffer)
cmake_minimum_required(VERSION 2.6.0)

# Sources.
SET(Blip_Buffer_SRCS
	Blip_Buffer.cpp
	)

# Build the library.
ADD_LIBRARY(Blip_Buffer STATIC ${Blip_Buffer_SRCS})
