/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * Config.hpp: Emulator configuration.                                     *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifndef __GENS_SDL_CONFIG_HPP__
#define __GENS_SDL_CONFIG_HPP__

#include "libgens/macros/common.h"

// Directory separator character.
#ifdef _WIN32
#define DIR_SEP_CHR '\\'
#else
#define DIR_SEP_CHR '/'
#endif

namespace GensSdl {

/**
 * Get the configuration directory.
 * @return Configuration directory, or nullptr on error.
 */
const utf8_str *getConfigDir(void);

}

#endif /* __GENS_SDL_CONFIG_HPP__ */
