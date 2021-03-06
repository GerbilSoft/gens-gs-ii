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

// Command line parameters.
#include "Options.hpp"

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
 * @param options Options.
 * @return Exit code.
 */
int CrazyEffectLoop::run(const Options *options)
{
	// Save options.
	// TODO: Make EmuLoop::run() non-virtual, save options there,
	// and then call protected virtual run_int()?
	CrazyEffectLoopPrivate *const d = d_func();
	d->options = options;

	// TODO: Move common code back to gens-sdl?

	// Initialize the SDL handlers.
	d->sdlHandler = new SdlHandler();
	if (d->sdlHandler->init_video() < 0)
		return EXIT_FAILURE;
	// No audio here.
	//if (d->sdlHandler->init_audio(options->sound_freq(), options->stereo()) < 0)
	//	return EXIT_FAILURE;
	d->vBackend = d->sdlHandler->vBackend();

	// Set the window title.
	d->sdlHandler->set_window_title("Gens/GS II [SDL]");

	// Check for startup messages.
	checkForStartupMessages();

	// Set frame timing.
	// NOTE: Always running the "Crazy Effect" at 60 fps.
	// TODO: Maybe 50 to compensate for dropout?
	d->setFrameTiming(60);
	// Disable frameskip for the "Crazy Effect".
	d->frameskip = false;

	// Create the "Crazy" Effect framebuffer.
	// Image size defaults to the full framebuffer,
	// so we don't have to worry about "stretch modes".
	d->crazyFb = new MdFb();
	d->crazyFb->setBpp(options->bpp());
	// Set the SDL video source.
	d->sdlHandler->set_video_source(d->crazyFb);

	// Create the "Crazy" Effect object.
	d->crazyEffect = new CrazyEffect();
	d->crazyEffect->setColorMask(CrazyEffect::CM_WHITE);

	// TODO: Move some more common stuff back to gens-sdl.cpp.
	d->running = true;
	while (d->running) {
		// Process the SDL event queue.
		processSdlEventQueue();
		if (!d->running) {
			// Emulation has stopped.
			break;
		}
		if (!d->running) {
			// Emulation has stopped.
			break;
		}
		if (d->paused.data) {
			// Emulation is paused.
			// Don't run any frames.
			// TODO: Wait for what would be the next frame?
			continue;
		}

		// Run a frame.
		// EventLoop::runFrame() handles frameskip timing.
		runFrame();
		yield();
		continue;
	}

	// Delete/unreference the "Crazy" Effect objects.
	// TODO: Move out of the private class and into local scope?
	d->crazyFb->unref();
	d->crazyFb = nullptr;
	delete d->crazyEffect;
	d->crazyEffect = nullptr;

	// NOTE: Deleting sdlHandler can cause crashes on Windows
	// due to the timer callback trying to post the semaphore
	// after it's been deleted.
	// Shut down the SDL functions manually.
	// FIXME: We're not using audio here, so maybe it won't crash?
	d->sdlHandler->end_video();
	d->vBackend = nullptr;

	// Done running the "Crazy" Effect loop.
	return 0;
}

/**
 * Run a normal frame.
 * This function is called by runFrame(),
 * and should be handled by running a full
 * frame with video and audio updates.
 */
void CrazyEffectLoop::runFullFrame(void)
{
	CrazyEffectLoopPrivate *const d = d_func();
	d->crazyEffect->run(d->crazyFb);
}

/**
 * Run a fast frame.
 * This function is called by runFrame() if the
 * system is lagging a bit, and should be handled
 * by running a frame with audio updates only.
 */
void CrazyEffectLoop::runFastFrame(void)
{
	// No fast frames here.
	// Run the full frame version.
	runFullFrame();
}

}
