/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * EmuLoop.hpp: Main emulation loop.                                       *
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

#ifndef __GENS_SDL_EMULOOP_HPP__
#define __GENS_SDL_EMULOOP_HPP__

namespace GensSdl {

// TODO: Make a base "main loop" class?

/**
 * Run the emulation loop.
 * @param rom_filename ROM filename. [TODO: Replace with options struct?]
 * @return Exit code.
 */
int EmuLoop(const char *rom_filename);

}

#endif /* __GENS_SDL_EMULOOP_HPP__ */
