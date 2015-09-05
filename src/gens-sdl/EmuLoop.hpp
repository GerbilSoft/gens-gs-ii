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

#include "EventLoop.hpp"
#include <string>

namespace LibZomg {
	class ZomgBase;
}

namespace GensSdl {

class EmuLoopPrivate;
class EmuLoop : public EventLoop
{
	public:
		EmuLoop();
		virtual ~EmuLoop();

	private:
		EVENT_LOOP_DECLARE_PRIVATE(EmuLoop)
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		EmuLoop(const EmuLoop &);
		EmuLoop &operator=(const EmuLoop &);

	public:
		/**
		 * Run the event loop.
		 * @param rom_filename ROM filename. [TODO: Replace with options struct?]
		 * @return Exit code.
		 */
		virtual int run(const char *rom_filename) final;

	protected:
		/**
		 * Process an SDL event.
		 * @param event SDL event.
		 * @return 0 if the event was handled; non-zero if it wasn't.
		 */
		virtual int processSdlEvent(const SDL_Event *event) final;
};

}

#endif /* __GENS_SDL_EMULOOP_HPP__ */
