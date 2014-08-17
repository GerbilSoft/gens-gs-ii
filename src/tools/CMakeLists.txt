PROJECT(tools)
cmake_minimum_required(VERSION 2.6)

# TODO: Header checks.
ADD_EXECUTABLE(mcd_pcm mcd_pcm.c)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(mcd_pcm)
