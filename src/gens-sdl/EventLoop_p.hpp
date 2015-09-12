/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * EventLoop_p.hpp: Event loop base class. (PRIVATE CLASS)                 *
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

#ifndef __GENS_SDL_EVENTLOOP_P_HPP__
#define __GENS_SDL_EVENTLOOP_P_HPP__

// OS-specific includes.
#ifdef _WIN32
// Windows
#include <windows.h>
// Win32 Unicode Translation Layer.
// Needed for proper Unicode filename support on Windows.
#include "libcompat/W32U/W32U_mini.h"
#include "libcompat/W32U/W32U_argv.h"
#else
// Linux, Unix, Mac OS X
#include <unistd.h>
#endif

// yield(), aka usleep(0) or Sleep(0)
#ifdef _WIN32
// Windows
#define yield() do { Sleep(0); } while (0)
#define usleep(usec) Sleep((DWORD)((usec) / 1000))
#else
// Linux, Unix, Mac OS X
#define yield() do { usleep(0); } while (0)
#endif

#include "libgens/Util/Timing.hpp"

// C++ includes.
#include <string>

namespace GensSdl {

class SdlHandler;
class VBackend;
class Options;

class EventLoopPrivate
{
	public:
		EventLoopPrivate();
		virtual ~EventLoopPrivate();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		EventLoopPrivate(const EventLoopPrivate &);
		EventLoopPrivate &operator=(const EventLoopPrivate &);

	public:
		/** Event handlers. **/

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

	public:
		/** SDL handler and video backend. **/
		SdlHandler *sdlHandler;
		VBackend *vBackend;

	public:
		/** Internal variables. **/
		bool running;
		bool frameskip;

		// Options.
		const Options *options;

		union paused_t {
			struct {
				uint8_t manual	: 1;	// Manual pause.
				uint8_t focus	: 1;	// Auto pause when focus is lost.
			};
			uint8_t data;
			// TODO: Add assign/copy constructors and operator==()?
		};
		paused_t paused;	// Current 'paused' value.

		// Window has been exposed.
		// Video should be updated if emulation is paused.
		bool exposed;

		class clks_t {
			public:
				// Reset frameskip timers.
				void reset(void) {
					// TODO: Reset timing's base?
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
		clks_t clks;

		// Last time the F1 message was displayed.
		// This is here to prevent the user from spamming
		// the display with the message.
		uint64_t lastF1time;

		// Microseconds per frame.
		unsigned int usec_per_frame;

		/**
		 * Set frame timing.
		 * This resets the frameskip timers.
		 * @param framerate Frame rate, e.g. 50 or 60.
		 */
		void setFrameTiming(int framerate);

	private:
		// Window title.
		// Usually the ROM's name.
		// Does not include modifiers like "[Paused]".
		std::string win_title;

	public:
		/**
		 * Update the window title.
		 * Call this function if the name doesn't
		 * need to be changed, but the state does.
		 */
		void updateWindowTitle(void);

		/**
		 * Update the window title.
		 * @param win_title New window title.
		 */
		void updateWindowTitle(const char *win_title);
};

}

#endif /* __GENS_SDL_EVENTLOOP_P_HPP__ */
