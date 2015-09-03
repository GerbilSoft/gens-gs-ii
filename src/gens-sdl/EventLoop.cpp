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

// C++ includes.
#include <string>
using std::string;

namespace GensSdl {

EventLoop::EventLoop()
	: m_running(false)
	, m_autoPause(false)
	, m_lastF1time(0)
{
	m_paused.data = 0;
}

EventLoop::~EventLoop()
{
	// Shut down SDL.
	// TODO
}

/**
 * Process an SDL event.
 * @param event SDL event.
 * @return 0 if the event was handled; non-zero if it wasn't.
 */
int EventLoop::processSdlEvent(const SDL_Event *event) {
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
			m_running = false;
			break;

		case SDL_KEYDOWN:
			// SDL keycodes nearly match GensKey.
			// TODO: Split out into a separate function?
			// TODO: Check for "no modifiers" for some keys?
			switch (event->key.keysym.sym) {
				case SDLK_ESCAPE:
					// Pause emulation.
					doPause();
					break;

				case SDLK_RETURN:
					// Check for Alt+Enter.
					if ((event->key.keysym.mod & KMOD_ALT) &&
					    !(event->key.keysym.mod & ~KMOD_ALT))
					{
						// Alt+Enter. Toggle fullscreen.
						m_sdlHandler->toggle_fullscreen();
					} else {
						// Not Alt+Enter.
						// We're not handling this event.
						ret = 1;
					}
					break;

				case SDLK_F1:
					// Show the "About" message.
					doAboutMessage();
					break;

				case SDLK_F9:
					// Fast Blur.
					doFastBlur();
					break;

				case SDLK_F12:
					// FIXME: TEMPORARY KEY BINDING for debugging.
					m_vBackend->setAspectRatioConstraint(!m_vBackend->aspectRatioConstraint());
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
					m_sdlHandler->resize_video(event->window.data1, event->window.data2);
					break;
				case SDL_WINDOWEVENT_EXPOSED:
					// Window has been exposed.
					// Tell the main loop to update video.
					m_exposed = true;
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					// If AutoPause is enabled, pause the emulator.
					if (m_autoPause) {
						doAutoPause(true);
					}
					break;
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					// If AutoPause is enabled, unpause the emulator.
					// TODO: Always run this, even if !autoPause?
					if (m_autoPause) {
						doAutoPause(false);
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
 * Toggle Fast Blur.
 */
void EventLoop::doFastBlur(void)
{
	bool fastBlur = !m_vBackend->fastBlur();
	m_vBackend->setFastBlur(fastBlur);

	// Show an OSD message.
	if (fastBlur) {
		m_vBackend->osd_print(1500, "Fast Blur enabled.");
	} else {
		m_vBackend->osd_print(1500, "Fast Blur disabled.");
	}
}

/**
 * Common pause processing function.
 * Called by doPause() and doAutoPause().
 */
void EventLoop::doPauseProcessing(void)
{
	bool manual = m_paused.manual;
	bool any = !!m_paused.data;
	// TODO: Option to disable the Paused Effect?
	// When enabled, it's only used for Manual Pause.
	m_vBackend->setPausedEffect(manual);

	// Reset the clocks and counters.
	m_clks.reset();
	// Pause audio.
	m_sdlHandler->pause_audio(any);

	// Update the window title.
	if (manual) {
		m_sdlHandler->set_window_title("Gens/GS II [SDL] [Paused]");
	} else {
		m_sdlHandler->set_window_title("Gens/GS II [SDL]");
	}
}

/**
 * Pause/unpause emulation.
 */
void EventLoop::doPause(void)
{
	m_paused.manual = !m_paused.manual;
	doPauseProcessing();
}

/**
 * Pause/unpause emulation in response to window focus changes.
 * @param lostFocus True if window lost focus; false if window gained focus.
 */
void EventLoop::doAutoPause(bool lostFocus)
{
	m_paused.focus = lostFocus;
	doPauseProcessing();
}

/**
 * Show the "About" message.
 */
void EventLoop::doAboutMessage(void)
{
       // TODO: OSD Gens logo as preview image, but with drop shadow disabled?
       const uint64_t curTime = m_clks.timing.getTime();
       if (m_lastF1time > 0 && (m_lastF1time + 5000000 > curTime)) {
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
       m_vBackend->osd_print(5000, ver_str.c_str());

       // Save the current time.
       m_lastF1time = curTime;
}

}
