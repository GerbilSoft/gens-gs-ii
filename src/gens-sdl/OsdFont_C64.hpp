/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * OsdFont_C64.hpp: C64 font for the Onscreen Display.                     *
 *                                                                         *
 * Copyright (c) 1982 Commodore International.                             *
 ***************************************************************************/

#ifndef __GENS_SDL_OSDFONT_C64_HPP__
#define __GENS_SDL_OSDFONT_C64_HPP__

#include <stdint.h>

namespace GensSdl {

// TODO: Standard base class with width and height info.
extern const uint8_t C64_charset_ASCII[256][8];

}

#endif /* __GENS_SDL_OSDFONT_C64_HPP__ */
