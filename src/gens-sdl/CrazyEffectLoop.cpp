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

// Reentrant functions.
// MUST be included before everything else due to
// _POSIX_SOURCE and _POSIX_C_SOURCE definitions.
#include "libcompat/reentrant.h"

#include "CrazyEffectLoop.hpp"
#include "gens-sdl.hpp"

#include "SdlHandler.hpp"
#include "VBackend.hpp"
using GensSdl::SdlHandler;
using GensSdl::VBackend;

// LibGens
#include "libgens/Rom.hpp"
#include "libgens/Util/MdFb.hpp"
using LibGens::Rom;
using LibGens::MdFb;

// "Crazy" Effect.
#include "libgens/Effects/CrazyEffect.hpp"
using LibGens::CrazyEffect;

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

namespace GensSdl {

static MdFb *crazyFb = nullptr;
static CrazyEffect *crazyEffect = nullptr;

/**
 * Run the "Crazy Effect" loop.
 * @return Exit code.
 */
int CrazyEffectLoop(void)
{
	// TODO: Move common code back to gens-sdl?

	// Initialize the SDL handlers.
	sdlHandler = new SdlHandler();
	if (sdlHandler->init_video() < 0)
		return EXIT_FAILURE;
	// No audio here.
	//if (sdlHandler->init_audio() < 0)
	//	return EXIT_FAILURE;
	vBackend = sdlHandler->vBackend();

	// Set the window title.
	sdlHandler->set_window_title("Gens/GS II [SDL]");

	// Check for startup messages.
	checkForStartupMessages();

	// Start the frame timer.
	// TODO: Region code?
	clks.reset();

	// Enable frameskip.
	frameskip = true;

	// Create the "Crazy" Effect framebuffer.
	// Image size defaults to the full framebuffer,
	// so we don't have to worry about "stretch modes".
	crazyFb = new MdFb();
	crazyFb->setBpp(MdFb::BPP_32);
	// Set the SDL video source.
	sdlHandler->set_video_source(crazyFb);

	// Create the "Crazy" Effect object.
	crazyEffect = new CrazyEffect();
	crazyEffect->setColorMask(CrazyEffect::CM_WHITE);

	// TODO: Move some more common stuff back to gens-sdl.cpp.
	while (running) {
		SDL_Event event;
		int ret;
		if (paused.data) {
			// Emulation is paused.
			if (!vBackend->has_osd_messages()) {
				// No OSD messages.
				// Wait for an SDL event.
				ret = SDL_WaitEvent(&event);
				if (ret) {
					processSdlEvent_common(&event);
				}
			}

			// Process OSD messages.
			vBackend->process_osd_messages();
		}
		if (!running)
			break;

		// Poll for SDL events, and wait for the queue
		// to empty. This ensures that we don't end up
		// only processing one event per frame.
		do {
			ret = SDL_PollEvent(&event);
			if (ret) {
				processSdlEvent_common(&event);
			}
		} while (running && ret != 0);
		if (!running)
			break;

		if (paused.data) {
			// Emulation is paused.
			// Only update video if the VBackend is dirty
			// or the SDL window has been exposed.
			sdlHandler->update_video_paused(exposed);

			// Don't run any frames.
			continue;
		}

		// Clear the 'exposed' flag.
		exposed = false;

		// New start time.
		clks.new_clk = timing.getTime();

		// Update the FPS counter.
		unsigned int fps_tmp = ((clks.new_clk - clks.fps_clk) & 0x3FFFFF);
		if (fps_tmp >= 1000000) {
			// More than 1 second has passed.
			clks.fps_clk = clks.new_clk;
			// FIXME: Just use abs() here.
			if (clks.frames_old > clks.frames) {
				clks.fps = (clks.frames_old - clks.frames);
			} else {
				clks.fps = (clks.frames - clks.frames_old);
			}
			clks.frames_old = clks.frames;

			// Update the window title.
			// TODO: Average the FPS over multiple seconds
			// and/or quarter-seconds.
			char win_title[256];
			snprintf(win_title, sizeof(win_title), "Gens/GS II [SDL] - %u fps", clks.fps);
			sdlHandler->set_window_title(win_title);
		}

		// Run the "Crazy" effect.
		// TODO: Use the frameskip code to limit frames?
		crazyEffect->run(crazyFb);
		sdlHandler->update_video();
		clks.frames++;
		yield();
		continue;
	}

	// Delete/unreference the "Crazy" Effect objects.
	crazyFb->unref();
	delete crazyEffect;

	// Done running the "Crazy" Effect loop.
	return 0;
}

}
