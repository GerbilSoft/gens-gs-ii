/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * Options.hpp: Command line option parser.                                *
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

#ifndef __GENS_SDL_OPTIONS_HPP__
#define __GENS_SDL_OPTIONS_HPP__

// C++ includes.
#include <string>

namespace GensSdl {

class Options
{
	public:
		Options();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		Options(const Options &);
		Options &operator=(const Options &);

	public:
		/**
		 * Reset all options to their default values.
		 */
		void reset(void);

		/**
		 * Parse command line arguments.
		 * @param argc
		 * @param argv
		 * @return 0 on success; non-zero on error.
		 */
		int parse(int argc, const char *argv[]);

	public:
		/**
		 * Is a ROM filename required?
		 * Set this before calling parse().
		 */
		bool rom_filename_required;

	public:
		/** Command line parameters. **/
		// TODO: Use accessors instead?
		std::string rom_filename;	// ROM to load.
		std::string tmss_rom_filename;	// TMSS ROM image.
		bool tmss_enabled;		// Enable TMSS?

		// Audio options.
		int sound_freq;			// Sound frequency.
		bool stereo;			// Stereo audio?

		// Emulation options.
		bool sprite_limits;		// Enable sprite limits?
		bool auto_fix_checksum;		// Auto fix checksum?

		// UI options.
		bool fps_counter;		// Enable FPS counter?
		bool auto_pause;		// Auto pause?
		bool paused_effect;		// Paused effect?
		uint8_t bpp;			// Color depth. (15, 16, 32)

		// Special run modes.
		bool run_crazy_effect;		// Run the Crazy Effect
						// instead of loading a ROM?
};

}

#endif /* __GENS_SDL_OPTIONS_HPP__ */
