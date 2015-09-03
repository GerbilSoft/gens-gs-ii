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

// TODO BEFORE COMMIT: Remove any more unnecessary includes.
// Also, after commit, convert emulation loops into classes?

// Reentrant functions.
// MUST be included before everything else due to
// _POSIX_SOURCE and _POSIX_C_SOURCE definitions.
#include "libcompat/reentrant.h"

#include <config.gens-sdl.h>
#include "gens-sdl.hpp"
#include <SDL.h>

// Configuration.
#include "Config.hpp"

#include "SdlHandler.hpp"
#include "VBackend.hpp"
using GensSdl::SdlHandler;
using GensSdl::VBackend;

// LibGens
#include "libgens/lg_main.hpp"
#include "libgens/lg_osd.h"
#include "libgens/Util/MdFb.hpp"
using LibGens::MdFb;
using LibGens::Timing;

// Main event loops.
#include "EmuLoop.hpp"
#include "CrazyEffectLoop.hpp"

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

// C includes. (C++ namespace)
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cerrno>

// C++ includes.
#include <string>
#include <vector>
using std::string;
using std::vector;

namespace GensSdl {

SdlHandler *sdlHandler = nullptr;
VBackend *vBackend = nullptr;

/** Command line parameters. **/
// TODO: Write a popt-based command line parser with
// a struct containing all of the options.
static const char *rom_filename = nullptr;
static bool autoPause = false;
// If true, don't emulate anything; just run the Crazy Effect.
static bool runCrazyEffect = false;

// Startup OSD message queue.
struct OsdStartup {
	int duration;
	const char *msg;
	int param;
};
static vector<OsdStartup> startup_queue;

// Last time the F1 message was displayed.
// This is here to prevent the user from spamming
// the display with the message.
static uint64_t lastF1time = 0;

/**
 * Onscreen Display handler.
 * @param osd_type OSD type.
 * @param param Parameter.
 */
static void gsdl_osd(OsdType osd_type, int param)
{
	// NOTE: We're not using any sort of translation system
	// in the SDL frontend, so we'll just use the plural form.
	// Most SRAM/EEPROM chips are larger than 1 byte, after all...
	const char *msg;
	switch (osd_type) {
		case OSD_SRAM_LOAD:
			msg = "SRAM loaded. (%d bytes)";
			break;
		case OSD_SRAM_SAVE:
			msg = "SRAM saved. (%d bytes)";
			break;
		case OSD_SRAM_AUTOSAVE:
			msg = "SRAM autosaved. (%d bytes)";
			break;
		case OSD_EEPROM_LOAD:
			msg = "EEPROM loaded. (%d bytes)";
			break;
		case OSD_EEPROM_SAVE:
			msg = "EEPROM saved. (%d bytes)";
			break;
		case OSD_EEPROM_AUTOSAVE:
			msg = "EEPROM autosaved. (%d bytes)";
			break;
		case OSD_PICO_PAGESET:
			msg = "Pico: Page set to page %d.";
			break;
		case OSD_PICO_PAGEUP:
			msg = "Pico: PgUp to page %d.";
			break;
		case OSD_PICO_PAGEDOWN:
			msg = "Pico: PgDn to page %d.";
			break;
		default:
			// Unknown OSD type.
			msg = nullptr;
			break;
	}

	if (msg != nullptr) {
		if (vBackend) {
			vBackend->osd_printf(1500, msg, param);
		} else {
			// SDL handler hasn't been created yet.
			// Store the message for later.
			OsdStartup startup;
			startup.duration = 1500;
			startup.msg = msg;
			startup.param = param;
			startup_queue.push_back(startup);
		}
	}
}

/**
 * Check for any OSD messages that were printed during startup.
 * These messages would have been printed before VBackend
 * was initialized, so they had to be temporarily stored.
 */
void checkForStartupMessages(void)
{
	if (!startup_queue.empty()) {
		for (int i = 0; i < (int)startup_queue.size(); i++) {
			const OsdStartup &startup = startup_queue.at(i);
			vBackend->osd_printf(startup.duration, startup.msg, startup.param);
		}
	}
}

// Timing object.
LibGens::Timing timing;

// Emulation state.
bool running = true;
paused_t paused;

// Enable frameskip.
bool frameskip = true;

// Window has been exposed.
// Video should be updated if emulation is paused.
bool exposed = false;

// Frameskip timers.
clks_t clks;

/**
 * Toggle Fast Blur.
 */
static void doFastBlur(void)
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
static void doPauseProcessing(void)
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
static void doPause(void)
{
	paused.manual = !paused.manual;
	doPauseProcessing();
}

/**
 * Pause/unpause emulation in response to window focus changes.
 * @param lostFocus True if window lost focus; false if window gained focus.
 */
static void doAutoPause(bool lostFocus)
{
	paused.focus = lostFocus;
	doPauseProcessing();
}

/**
 * Show the "About" message.
 */
static void doAboutMessage(void)
{
       // TODO: OSD Gens logo as preview image, but with drop shadow disabled?
       const uint64_t curTime = timing.getTime();
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

/**
 * Process an SDL event.
 * EmuLoop processes SDL events first; if it ends up
 * with an event that it can't handle, it goes here.
 * @param event SDL event.
 * @return 0 if the event was handled; non-zero if it wasn't.
 */
int processSdlEvent_common(const SDL_Event *event) {
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
			running = false;
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
						sdlHandler->toggle_fullscreen();
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
					vBackend->setAspectRatioConstraint(!vBackend->aspectRatioConstraint());
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
					sdlHandler->resize_video(event->window.data1, event->window.data2);
					break;
				case SDL_WINDOWEVENT_EXPOSED:
					// Window has been exposed.
					// Tell the main loop to update video.
					exposed = true;
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					// If AutoPause is enabled, pause the emulator.
					if (autoPause) {
						doAutoPause(true);
					}
					break;
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					// If AutoPause is enabled, unpause the emulator.
					// TODO: Always run this, even if !autoPause?
					if (autoPause) {
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
 * Run the emulator.
 */
int run(void)
{
#ifdef _WIN32
	// Reference: http://sdl.beuc.net/sdl.wiki/FAQ_Console
	// TODO: Set console as UTF-8.
	freopen("CON", "w", stdout);
	freopen("CON", "w", stderr);
#endif /* _WIN32 */

	// Initialize LibGens.
	LibGens::Init();

	// Register the LibGens OSD handler.
	lg_set_osd_fn(gsdl_osd);

	int ret;
	if (runCrazyEffect) {
		// Run the Crazy Effect.
		ret = CrazyEffectLoop();
	} else {
		// Start the emulation loop.
		ret = EmuLoop(rom_filename);
	}

	// COMMIT CHECK: How many linebreaks were here?

	// Pause audio and wait 50ms for SDL to catch up.
	sdlHandler->pause_audio(true);
	usleep(50000);

	// NULL out the VBackend before shutting down SDL.
	vBackend = nullptr;

	// Unregister the LibGens OSD handler.
	lg_set_osd_fn(nullptr);

	if (sdlHandler) {
		// NOTE: Deleting sdlHandler can cause crashes on Windows
		// due to the timer callback trying to post the semaphore
		// after it's been deleted.
		// Shut down the SDL functions manually.
		sdlHandler->end_audio();
		sdlHandler->end_video();
		//delete sdlHandler;
	}

	// ...and we're done here.
	return ret;
}

}

// Don't use SDL_main.
#undef main
int main(int argc, char *argv[])
{
#ifdef _WIN32
	// Convert command line parameters to UTF-8.
	if (W32U_GetArgvU(&argc, &argv, nullptr) != 0) {
		// ERROR!
		return EXIT_FAILURE;
	}
#endif /* _WIN32 */

	// TODO: Use popt; don't require a ROM filename if
	// using "Crazy" Effect mode.
	if (argc < 2) {
		fprintf(stderr, "usage: %s [rom filename]\n", argv[0]);
		return EXIT_FAILURE;
	}
	GensSdl::rom_filename = argv[1];

	// Make sure we have a valid configuration directory.
	if (GensSdl::getConfigDir().empty()) {
		fprintf(stderr, "*** WARNING: Could not find a usable configuration directory.\n"
				"Save functionality will be disabled.\n\n");
	}

	// Initialize SDL.
	int ret = SDL_Init(0);
	if (ret < 0) {
		fprintf(stderr, "SDL initialization failed: %d - %s\n",
			ret, SDL_GetError());
		return EXIT_FAILURE;
	}

	return GensSdl::run();
}
