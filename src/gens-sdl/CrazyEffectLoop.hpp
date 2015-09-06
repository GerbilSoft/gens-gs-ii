/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * CrazyEffectLoop.hpp: "Crazy" Effect loop.                               *
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

#ifndef __GENS_SDL_CRAZYEFFECTLOOP_HPP__
#define __GENS_SDL_CRAZYEFFECTLOOP_HPP__

#include "EventLoop.hpp"

namespace GensSdl {

class CrazyEffectLoopPrivate;
class CrazyEffectLoop : public EventLoop
{
	public:
		CrazyEffectLoop();
		virtual ~CrazyEffectLoop();

	private:
		EVENT_LOOP_DECLARE_PRIVATE(CrazyEffectLoop)
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		CrazyEffectLoop(const CrazyEffectLoop &);
		CrazyEffectLoop &operator=(const CrazyEffectLoop &);

	public:
		/**
		 * Run the event loop.
		 * @param rom_filename ROM filename. [TODO: Replace with options struct?]
		 * @return Exit code.
		 */
		virtual int run(const char *rom_filename) final;

	protected:
		// TODO: Move to CrazyEffectLoopPrivate?

		/**
		 * Run a normal frame.
		 * This function is called by runFrame(),
		 * and should be handled by running a full
		 * frame with video and audio updates.
		 */
		virtual void runFullFrame(void) final;

		/**
		 * Run a fast frame.
		 * This function is called by runFrame() if the
		 * system is lagging a bit, and should be handled
		 * by running a frame with audio updates only.
		 */
		virtual void runFastFrame(void) final;
};

}

#endif /* __GENS_SDL_CRAZYEFFECTLOOP_HPP__ */
