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

// MdFb
#include "libgens/Util/MdFb.hpp"

// C++ includes.
#include <string>

namespace GensSdl {

class OptionsPrivate;
class Options
{
	public:
		Options();
		~Options();

	private:
		friend class OptionsPrivate;
		OptionsPrivate *const d;
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
		/** Options set by the caller. **/

		/**
		 * Is a ROM filename required?
		 * @return True if a ROM filename is required; false if not.
		 */
		bool is_rom_filename_required(void) const;

		/**
		 * Set if a ROM filename should be required.
		 * @param rom_filename_required True if a ROM filename is required; false if not.
		 */
		void set_rom_filename_required(bool rom_filename_required);

	public:
		/** Command line parameters. **/

		/**
		 * Get the filename of the ROM to load.
		 */
		std::string rom_filename(void) const;

		/**
		 * Get the filename of the TMSS ROM to load.
		 */
		std::string tmss_rom_filename(void) const;

		/**
		 * Is TMSS enabled?
		 * This option is implied by the presence of a TMSS ROM filename.
		 * @return True if TMSS is enabled; false if not.
		 */
		bool is_tmss_enabled(void) const;

		/** Audio options. **/

		/**
		 * Get the requested sound frequency.
		 * @return Sound frequency.
		 */
		int sound_freq(void) const;

		/**
		 * Use stereo audio?
		 * @return True for stereo; false for monaural.
		 */
		bool stereo(void) const;

		/** Emulation options. **/

		/**
		 * Enable sprite limits?
		 * @return True to enable; false to disable.
		 */
		bool sprite_limits(void) const;

		/**
		 * Automatically fix checksums?
		 * @return True to auto-fix; false to not.
		 */
		bool auto_fix_checksum(void) const;

		/** UI options. **/

		/**
		 * Enable the FPS counter?
		 * @return True to enable; false to disable.
		 */
		bool fps_counter(void) const;

		/**
		 * Automatically pause the emulator when focus is lost?
		 * @return True to automatically pause; false to not.
		 */
		bool auto_pause(void) const;

		/**
		 * Use the paused effect when the emulator is manually paused?
		 * @return True to use the paused effect; false to not.
		 */
		bool paused_effect(void) const;

		/**
		 * Color depth to use.
		 * @return Color depth.
		 */
		LibGens::MdFb::ColorDepth bpp(void) const;

		/** Special run modes. **/

		/**
		 * Run the Crazy Effect instead of loading a ROM?
		 * @return True to run the Crazy Effect.
		 */
		bool run_crazy_effect(void) const;
};

}

#endif /* __GENS_SDL_OPTIONS_HPP__ */
