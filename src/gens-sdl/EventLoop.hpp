/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * EventLoop.hpp: Event loop base class.                                   *
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

#ifndef __GENS_SDL_EVENTLOOP_HPP__
#define __GENS_SDL_EVENTLOOP_HPP__

#include "libgens/Util/Timing.hpp"
#include <SDL.h>

namespace GensSdl {

class SdlHandler;
class VBackend;

class EventLoop
{
	public:
		EventLoop();
		virtual ~EventLoop();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		EventLoop(const EventLoop &);
		EventLoop &operator=(const EventLoop &);

	public:
		/**
		 * Run the event loop.
		 * @param rom_filename ROM filename. [TODO: Replace with options struct?]
		 * @return Exit code.
		 */
		virtual int run(const char *rom_filename) = 0;

	protected:
		/**
		 * Process an SDL event.
		 * @param event SDL event.
		 * @return 0 if the event was handled; non-zero if it wasn't.
		 */
		virtual int processSdlEvent(const SDL_Event *event);

		/**
		 * Toggle Fast Blur.
		 */
		void doFastBlur(void);

		/**
		 * Common pause processing function.
		 * Called by doPause() and doAutoPause().
		 */
		void doPauseProcessing(void);

		/**
		 * Pause/unpause emulation.
		 */
		void doPause(void);

		/**
		 * Pause/unpause emulation in response to window focus changes.
		 * @param lostFocus True if window lost focus; false if window gained focus.
		 */
		void doAutoPause(bool lostFocus);

		/**
		 * Show the "About" message.
		 */
		void doAboutMessage(void);

	protected:
		// TODO: Move to a private class?
		bool m_running;
		bool m_frameskip;

		union paused_t {
			struct {
				uint8_t manual	: 1;	// Manual pause.
				uint8_t focus	: 1;	// Auto pause when focus is lost.
			};
			uint8_t data;
		};
		paused_t m_paused;

		// Automatically pause when the window loses focus?
		bool m_autoPause;

		// Window has been exposed.
		// Video should be updated if emulation is paused.
		bool m_exposed;

		class clks_t {
			public:
				// Reset frameskip timers.
				void reset(void) {
					start_clk = timing.getTime();
					old_clk = start_clk;
					fps_clk = start_clk;
					fps_clk = start_clk;
					new_clk = start_clk;
					usec_frameskip = 0;

					// Frame counter.
					frames = 0;
					frames_old = 0;
					fps = 0;
				}

				// Timing object.
				LibGens::Timing timing;

				// Clocks.
				uint64_t start_clk;
				uint64_t old_clk;
				uint64_t fps_clk;
				uint64_t new_clk;
				// Microsecond counter for frameskip.
				uint64_t usec_frameskip;

				// Frame counters.
				unsigned int frames;
				unsigned int frames_old;
				unsigned int fps;	// TODO: float or double?
		};
		clks_t m_clks;

		// Last time the F1 message was displayed.
		// This is here to prevent the user from spamming
		// the display with the message.
		uint64_t m_lastF1time;

	public:
		// TODO: Make this protected.
		SdlHandler *m_sdlHandler;
		VBackend *m_vBackend;
};

}

#endif /* __GENS_SDL_EVENTLOOP_HPP__ */
