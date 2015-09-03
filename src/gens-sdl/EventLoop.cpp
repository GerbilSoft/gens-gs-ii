/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * EventLoop.cpp: Event loop base class.                                   *
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

#include "EventLoop.hpp"
#include "gens-sdl.hpp"

// Configuration.
#include "Config.hpp"

#include "SdlHandler.hpp"
#include "VBackend.hpp"
using GensSdl::SdlHandler;
using GensSdl::VBackend;

// LibGens
#include "libgens/lg_main.hpp"
#include "libgens/Util/MdFb.hpp"
using LibGens::MdFb;

// C++ includes.
#include <string>
using std::string;

#include "EventLoop_p.hpp"
namespace GensSdl {

/** EventLoopPrivate **/

EventLoopPrivate::EventLoopPrivate()
	: sdlHandler(nullptr)
	, vBackend(nullptr)
	, running(false)
	, frameskip(true)
	, autoPause(false)
	, exposed(false)
	, lastF1time(0)
{
	paused.data = 0;
}

EventLoopPrivate::~EventLoopPrivate()
{
	// TODO: Shut down SDL if it's still running?
}

/**
 * Toggle Fast Blur.
 */
void EventLoopPrivate::doFastBlur(void)
{
	bool fastBlur = !vBackend->fastBlur();
	vBackend->setFastBlur(fastBlur);

	// Show an OSD message.
	if (fastBlur) {
		vBackend->osd_print(1500, "Fast Blur enabled.");
	} else {
		vBackend->osd_print(1500, "Fast Blur disabled.");
	}
}

/**
 * Common pause processing function.
 * Called by doPause() and doAutoPause().
 */
void EventLoopPrivate::doPauseProcessing(void)
{
	bool manual = paused.manual;
	bool any = !!paused.data;
	// TODO: Option to disable the Paused Effect?
	// When enabled, it's only used for Manual Pause.
	vBackend->setPausedEffect(manual);

	// Reset the clocks and counters.
	clks.reset();
	// Pause audio.
	sdlHandler->pause_audio(any);

	// Update the window title.
	if (manual) {
		sdlHandler->set_window_title("Gens/GS II [SDL] [Paused]");
	} else {
		sdlHandler->set_window_title("Gens/GS II [SDL]");
	}
}

/**
 * Pause/unpause emulation.
 */
void EventLoopPrivate::doPause(void)
{
	paused.manual = !paused.manual;
	doPauseProcessing();
}

/**
 * Pause/unpause emulation in response to window focus changes.
 * @param lostFocus True if window lost focus; false if window gained focus.
 */
void EventLoopPrivate::doAutoPause(bool lostFocus)
{
	paused.focus = lostFocus;
	doPauseProcessing();
}

/**
 * Show the "About" message.
 */
void EventLoopPrivate::doAboutMessage(void)
{
       // TODO: OSD Gens logo as preview image, but with drop shadow disabled?
       const uint64_t curTime = clks.timing.getTime();
       if (lastF1time > 0 && (lastF1time + 5000000 > curTime)) {
               // Timer hasn't expired.
               // Don't show the message.
               return;
       }

       // Version string.
       string ver_str;
       ver_str.reserve(512);
       ver_str = "Gens/GS II - SDL2 frontend\n";
       ver_str += "Version " + string(LibGens::version);
       if (LibGens::version_vcs) {
               ver_str += " (" + string(LibGens::version_vcs) + ')';
       }
       ver_str += '\n';
       if (LibGens::version_desc) {
               ver_str += string(LibGens::version_desc) + '\n';
       }
#if !defined(GENS_ENABLE_EMULATION)
	ver_str += "[NO-EMULATION BUILD]\n";
#endif
	ver_str += "(c) 2008-2015 by David Korth.";

       // Show a new message.
       vBackend->osd_print(5000, ver_str.c_str());

       // Save the current time.
       lastF1time = curTime;
}

/** EventLoop **/

EventLoop::EventLoop(EventLoopPrivate *d)
	: d_ptr(d)
{ }

EventLoop::~EventLoop()
{
	delete d_ptr;
}

/**
 * Get the VBackend.
 * @return VBackend.
 */
VBackend *EventLoop::vBackend(void) const
{
	return d_ptr->vBackend;
}

/**
 * Process an SDL event.
 * @param event SDL event.
 * @return 0 if the event was handled; non-zero if it wasn't.
 */
int EventLoop::processSdlEvent(const SDL_Event *event)
{
	// NOTE: Some keys are processed in specific event loops
	// instead of here because they only apply if a ROM is loaded.
	// gens-qt4 won't make a distinction, since it can run with
	// both a ROM loaded and not loaded, and you can open and close
	// ROMs without restarting the program, so it has to be able
	// to switch modes while allowing options to be change both
	// when a ROM is loaded and when no ROM is loaded.
	// In essence, it combines both EmuLoop and CrazyEffectLoop
	// while maintaining the state of various emulation options.

	int ret = 0;
	switch (event->type) {
		case SDL_QUIT:
			d_ptr->running = false;
			break;

		case SDL_KEYDOWN:
			// SDL keycodes nearly match GensKey.
			// TODO: Split out into a separate function?
			// TODO: Check for "no modifiers" for some keys?
			switch (event->key.keysym.sym) {
				case SDLK_ESCAPE:
					// Pause emulation.
					d_ptr->doPause();
					break;

				case SDLK_RETURN:
					// Check for Alt+Enter.
					if ((event->key.keysym.mod & KMOD_ALT) &&
					    !(event->key.keysym.mod & ~KMOD_ALT))
					{
						// Alt+Enter. Toggle fullscreen.
						d_ptr->sdlHandler->toggle_fullscreen();
					} else {
						// Not Alt+Enter.
						// We're not handling this event.
						ret = 1;
					}
					break;

				case SDLK_F1:
					// Show the "About" message.
					d_ptr->doAboutMessage();
					break;

				case SDLK_F9:
					// Fast Blur.
					d_ptr->doFastBlur();
					break;

				case SDLK_F12:
					// FIXME: TEMPORARY KEY BINDING for debugging.
					d_ptr->vBackend->setAspectRatioConstraint(!d_ptr->vBackend->aspectRatioConstraint());
					break;

				default:
					// Event not handled.
					ret = 1;
					break;
			}
			break;

		case SDL_WINDOWEVENT:
			switch (event->window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					// Resize the video renderer.
					d_ptr->sdlHandler->resize_video(event->window.data1, event->window.data2);
					break;
				case SDL_WINDOWEVENT_EXPOSED:
					// Window has been exposed.
					// Tell the main loop to update video.
					d_ptr->exposed = true;
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					// If AutoPause is enabled, pause the emulator.
					if (d_ptr->autoPause) {
						d_ptr->doAutoPause(true);
					}
					break;
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					// If AutoPause is enabled, unpause the emulator.
					// TODO: Always run this, even if !autoPause?
					if (d_ptr->autoPause) {
						d_ptr->doAutoPause(false);
					}
					break;
				default:
					// Event not handled.
					ret = 1;
					break;
			}
			break;

		default:
			// Event not handled.
			ret = 1;
			break;
	}

	return ret;
}

/**
 * Process the SDL event queue.
 * If emulation is paused and the OSD message,
 * list is empty, SDL_WaitEvent() will be used
 * to wait for the next event.
 */
void EventLoop::processSdlEventQueue(void)
{
	SDL_Event event;
	int ret;
	if (d_ptr->paused.data) {
		// Emulation is paused.
		if (!d_ptr->vBackend->has_osd_messages()) {
			// No OSD messages.
			// Wait for an SDL event.
			ret = SDL_WaitEvent(&event);
			if (ret) {
				processSdlEvent(&event);
			}
		}

		// Process OSD messages.
		d_ptr->vBackend->process_osd_messages();
	}
	if (!d_ptr->running)
		return;

	// Poll for SDL events, and wait for the queue
	// to empty. This ensures that we don't end up
	// only processing one event per frame.
	do {
		ret = SDL_PollEvent(&event);
		if (ret) {
			processSdlEvent(&event);
		}
	} while (d_ptr->running && ret != 0);
	if (!d_ptr->running)
		return;

	if (d_ptr->paused.data) {
		// Emulation is paused.
		// Only update video if the VBackend is dirty
		// or the SDL window has been exposed.
		d_ptr->sdlHandler->update_video_paused(d_ptr->exposed);
	}

	// Clear the 'exposed' flag.
	d_ptr->exposed = false;
}

}
