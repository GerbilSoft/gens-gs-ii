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

#include "EventLoop_p.hpp"
namespace GensSdl {

class CrazyEffectLoopPrivate : public EventLoopPrivate
{
	public:
		CrazyEffectLoopPrivate();
		virtual ~CrazyEffectLoopPrivate();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		CrazyEffectLoopPrivate(const CrazyEffectLoopPrivate &);
		CrazyEffectLoopPrivate &operator=(const CrazyEffectLoopPrivate &);

	public:
		MdFb *crazyFb;
		CrazyEffect *crazyEffect;
};

/** CrazyEffectLoopPrivate **/

CrazyEffectLoopPrivate::CrazyEffectLoopPrivate()
	: crazyFb(nullptr)
	, crazyEffect(nullptr)
{ }

CrazyEffectLoopPrivate::~CrazyEffectLoopPrivate()
{
	if (crazyFb) {
		crazyFb->unref();
		crazyFb = nullptr;
	}
	delete crazyEffect;
}

/** CrazyEffectLoop **/

CrazyEffectLoop::CrazyEffectLoop()
	: EventLoop(new CrazyEffectLoopPrivate())
{ }

CrazyEffectLoop::~CrazyEffectLoop()
{ }

/**
 * Run the event loop.
 * @param rom_filename ROM filename. [TODO: Replace with options struct?]
 * @return Exit code.
 */
int CrazyEffectLoop::run(const char *rom_filename)
{
	// rom_filename isn't used here.
	((void)rom_filename);

	// TODO: Move common code back to gens-sdl?

	// Initialize the SDL handlers.
	CrazyEffectLoopPrivate *const d = d_func();
	d->sdlHandler = new SdlHandler();
	if (d->sdlHandler->init_video() < 0)
		return EXIT_FAILURE;
	// No audio here.
	//if (d->sdlHandler->init_audio() < 0)
	//	return EXIT_FAILURE;
	d->vBackend = d->sdlHandler->vBackend();

	// Set the window title.
	d->sdlHandler->set_window_title("Gens/GS II [SDL]");

	// Check for startup messages.
	checkForStartupMessages();

	// Start the frame timer.
	// TODO: Region code?
	d->clks.reset();

	// Create the "Crazy" Effect framebuffer.
	// Image size defaults to the full framebuffer,
	// so we don't have to worry about "stretch modes".
	d->crazyFb = new MdFb();
	d->crazyFb->setBpp(MdFb::BPP_32);
	// Set the SDL video source.
	d->sdlHandler->set_video_source(d->crazyFb);

	// Create the "Crazy" Effect object.
	d->crazyEffect = new CrazyEffect();
	d->crazyEffect->setColorMask(CrazyEffect::CM_WHITE);

	// TODO: Move some more common stuff back to gens-sdl.cpp.
	d->running = true;
	while (d->running) {
		SDL_Event event;
		int ret;
		if (d->paused.data) {
			// Emulation is paused.
			if (!d->vBackend->has_osd_messages()) {
				// No OSD messages.
				// Wait for an SDL event.
				ret = SDL_WaitEvent(&event);
				if (ret) {
					processSdlEvent(&event);
				}
			}

			// Process OSD messages.
			d->vBackend->process_osd_messages();
		}
		if (!d->running)
			break;

		// Poll for SDL events, and wait for the queue
		// to empty. This ensures that we don't end up
		// only processing one event per frame.
		do {
			ret = SDL_PollEvent(&event);
			if (ret) {
				processSdlEvent(&event);
			}
		} while (d->running && ret != 0);
		if (!d->running)
			break;

		if (d->paused.data) {
			// Emulation is paused.
			// Only update video if the VBackend is dirty
			// or the SDL window has been exposed.
			d->sdlHandler->update_video_paused(d->exposed);

			// Don't run any frames.
			continue;
		}

		// Clear the 'exposed' flag.
		d->exposed = false;

		// New start time.
		d->clks.new_clk = d->clks.timing.getTime();

		// Update the FPS counter.
		unsigned int fps_tmp = ((d->clks.new_clk - d->clks.fps_clk) & 0x3FFFFF);
		if (fps_tmp >= 1000000) {
			// More than 1 second has passed.
			d->clks.fps_clk = d->clks.new_clk;
			// FIXME: Just use abs() here.
			if (d->clks.frames_old > d->clks.frames) {
				d->clks.fps = (d->clks.frames_old - d->clks.frames);
			} else {
				d->clks.fps = (d->clks.frames - d->clks.frames_old);
			}
			d->clks.frames_old = d->clks.frames;

			// Update the window title.
			// TODO: Average the FPS over multiple seconds
			// and/or quarter-seconds.
			char win_title[256];
			snprintf(win_title, sizeof(win_title), "Gens/GS II [SDL] - %u fps", d->clks.fps);
			d->sdlHandler->set_window_title(win_title);
		}

		// Run the "Crazy" effect.
		// TODO: Use the frameskip code to limit frames?
		d->crazyEffect->run(d->crazyFb);
		d->sdlHandler->update_video();
		d->clks.frames++;
		yield();
		continue;
	}

	// Delete/unreference the "Crazy" Effect objects.
	// TODO: Move out of the private class and into local scope?
	d->crazyFb->unref();
	d->crazyFb = nullptr;
	delete d->crazyEffect;
	d->crazyEffect = nullptr;

	// Done running the "Crazy" Effect loop.
	return 0;
}

}
