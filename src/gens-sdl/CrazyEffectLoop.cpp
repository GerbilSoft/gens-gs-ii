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

class CrazyEffectLoopPrivate
{
	public:
		CrazyEffectLoopPrivate();
		~CrazyEffectLoopPrivate();

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
	: d(new CrazyEffectLoopPrivate())
{ }

CrazyEffectLoop::~CrazyEffectLoop()
{
	delete d;
}

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
	m_sdlHandler = new SdlHandler();
	if (m_sdlHandler->init_video() < 0)
		return EXIT_FAILURE;
	// No audio here.
	//if (m_sdlHandler->init_audio() < 0)
	//	return EXIT_FAILURE;
	m_vBackend = m_sdlHandler->vBackend();

	// Set the window title.
	m_sdlHandler->set_window_title("Gens/GS II [SDL]");

	// Check for startup messages.
	checkForStartupMessages();

	// Start the frame timer.
	// TODO: Region code?
	m_clks.reset();

	// Create the "Crazy" Effect framebuffer.
	// Image size defaults to the full framebuffer,
	// so we don't have to worry about "stretch modes".
	d->crazyFb = new MdFb();
	d->crazyFb->setBpp(MdFb::BPP_32);
	// Set the SDL video source.
	m_sdlHandler->set_video_source(d->crazyFb);

	// Create the "Crazy" Effect object.
	d->crazyEffect = new CrazyEffect();
	d->crazyEffect->setColorMask(CrazyEffect::CM_WHITE);

	// TODO: Move some more common stuff back to gens-sdl.cpp.
	m_running = true;
	while (m_running) {
		SDL_Event event;
		int ret;
		if (m_paused.data) {
			// Emulation is paused.
			if (!m_vBackend->has_osd_messages()) {
				// No OSD messages.
				// Wait for an SDL event.
				ret = SDL_WaitEvent(&event);
				if (ret) {
					processSdlEvent(&event);
				}
			}

			// Process OSD messages.
			m_vBackend->process_osd_messages();
		}
		if (!m_running)
			break;

		// Poll for SDL events, and wait for the queue
		// to empty. This ensures that we don't end up
		// only processing one event per frame.
		do {
			ret = SDL_PollEvent(&event);
			if (ret) {
				processSdlEvent(&event);
			}
		} while (m_running && ret != 0);
		if (!m_running)
			break;

		if (m_paused.data) {
			// Emulation is paused.
			// Only update video if the VBackend is dirty
			// or the SDL window has been exposed.
			m_sdlHandler->update_video_paused(m_exposed);

			// Don't run any frames.
			continue;
		}

		// Clear the 'exposed' flag.
		m_exposed = false;

		// New start time.
		m_clks.new_clk = m_clks.timing.getTime();

		// Update the FPS counter.
		unsigned int fps_tmp = ((m_clks.new_clk - m_clks.fps_clk) & 0x3FFFFF);
		if (fps_tmp >= 1000000) {
			// More than 1 second has passed.
			m_clks.fps_clk = m_clks.new_clk;
			// FIXME: Just use abs() here.
			if (m_clks.frames_old > m_clks.frames) {
				m_clks.fps = (m_clks.frames_old - m_clks.frames);
			} else {
				m_clks.fps = (m_clks.frames - m_clks.frames_old);
			}
			m_clks.frames_old = m_clks.frames;

			// Update the window title.
			// TODO: Average the FPS over multiple seconds
			// and/or quarter-seconds.
			char win_title[256];
			snprintf(win_title, sizeof(win_title), "Gens/GS II [SDL] - %u fps", m_clks.fps);
			m_sdlHandler->set_window_title(win_title);
		}

		// Run the "Crazy" effect.
		// TODO: Use the frameskip code to limit frames?
		d->crazyEffect->run(d->crazyFb);
		m_sdlHandler->update_video();
		m_clks.frames++;
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
