/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * gens-sdl.hpp: Entry point.                                              *
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

#ifndef __GENS_SDL_HPP__
#define __GENS_SDL_HPP__
 
// TODO: Don't include SDL.h here?
#include <SDL.h>

// LibGens::Timing
// TODO: Split clks_t into a separate header file?
#include "libgens/Util/Timing.hpp"

namespace GensSdl {

class SdlHandler;
extern SdlHandler *sdlHandler;

class VBackend;
extern VBackend *vBackend;

extern bool running;

union paused_t {
	struct {
		uint8_t manual	: 1;	// Manual pause.
		uint8_t focus	: 1;	// Auto pause when focus is lost.
	};
	uint8_t data;
};
extern paused_t paused;

// Enable frameskip.
extern bool frameskip;

// Window has been exposed.
// Video should be updated if emulation is paused.
extern bool exposed;

// Timing object.
extern LibGens::Timing timing;

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
extern clks_t clks;

/**
 * Check for any OSD messages that were printed during startup.
 * These messages would have been printed before VBackend
 * was initialized, so they had to be temporarily stored.
 */
void checkForStartupMessages(void);

/**
 * Process an SDL event.
 * EmuLoop processes SDL events first; if it ends up
 * with an event that it can't handle, it goes here.
 * @param event SDL event.
 * @return 0 if the event was handled; non-zero if it wasn't.
 */
int processSdlEvent_common(const SDL_Event *event);

}

#endif /* __GENS_SDL_HPP__ */
