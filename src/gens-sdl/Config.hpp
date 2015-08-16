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

// Directory separator character.
#ifdef _WIN32
#define DIR_SEP_CHR '\\'
#else
#define DIR_SEP_CHR '/'
#endif

// C++ includes.
#include <string>

namespace LibGens {
	class MdFb;
	class Rom;
}

namespace GensSdl {

/**
 * Get the configuration directory.
 * @param subdir [in, opt] If not null, append a subdirectory.
 * @return Configuration directory, or empty string on error.
 */
std::string getConfigDir(const char *subdir = nullptr);

/**
 * Get a savestate filename.
 * @param rom ROM for the savestate.
 * @param saveSlot Save slot number. (0-9)
 * @return Savestate filename.
 */
std::string getSavestateFilename(const LibGens::Rom *rom, int saveSlot);

/**
 * Take a screenshot.
 * @param fb	[in] MdFb.
 * @param rom	[in] ROM object.
 * @return Screenshot number on success; negative errno on error.
 */
int doScreenShot(const LibGens::MdFb *fb, const LibGens::Rom *rom);

}

#endif /* __GENS_SDL_CONFIG_HPP__ */
