/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * gens-sdl.cpp: Entry point and main event loop.                          *
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

#include "SdlHandler.hpp"
#include "Config.hpp"
using GensSdl::SdlHandler;

// LibGens
#include "libgens/lg_main.hpp"
#include "libgens/lg_osd.h"
#include "libgens/Rom.hpp"
#include "libgens/MD/EmuMD.hpp"
#include "libgens/Util/MdFb.hpp"
#include "libgens/Util/Timing.hpp"
using LibGens::Rom;
using LibGens::EmuContext;
using LibGens::EmuMD;
using LibGens::MdFb;
using LibGens::Timing;

// LibGensKeys
#include "libgens/IO/IoManager.hpp"
#include "libgens/macros/common.h"
#include "libgenskeys/KeyManager.hpp"
#include "libgenskeys/GensKey_t.h"
using LibGens::IoManager;
using LibGensKeys::KeyManager;

// OS-specific includes.
#ifdef _WIN32
// Windows
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "libgens/Win32/W32U_mini.h"
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

// C++ includes.
#include <string>
using std::string;

#include <SDL.h>

static SdlHandler *sdlHandler = nullptr;
static Rom *rom = nullptr;
static EmuMD *context = nullptr;
static const char *rom_filename = nullptr;

static KeyManager *keyManager = nullptr;
// NOTE: Using SDL keycodes here.
// TODO: Proper SDL to GensKey conversion.
static const GensKey_t keyMap[] = {
	SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT,	// UDLR
	SDLK_s, SDLK_d, SDLK_a, SDLK_RETURN,		// BCAS
	SDLK_e, SDLK_w, SDLK_q, SDLK_RSHIFT		// ZYXM
};

namespace GensSdl {

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
	switch (osd_type) {
		case OSD_SRAM_LOAD:
			printf("SRAM loaded. (%d bytes)\n", param);
			break;
		case OSD_SRAM_SAVE:
			printf("SRAM saved. (%d bytes)\n", param);
			break;
		case OSD_SRAM_AUTOSAVE:
			printf("SRAM autosaved. (%d bytes)\n", param);
			break;
		case OSD_EEPROM_LOAD:
			printf("EEPROM loaded. (%d bytes)\n", param);
			break;
		case OSD_EEPROM_SAVE:
			printf("EEPROM saved. (%d bytes)\n", param);
			break;
		case OSD_EEPROM_AUTOSAVE:
			printf("EEPROM autosaved. (%d bytes)\n", param);
			break;
		default:
			// Unknown OSD type.
			break;
	}
}

}

// Don't use SDL_main.
#undef main
int main(int argc, char *argv[])
{
#ifdef _WIN32
	W32U_Init();
#endif /* _WIN32 */

	if (argc < 2) {
		fprintf(stderr, "usage: %s [rom filename]\n", argv[0]);
		return EXIT_FAILURE;
	}
	rom_filename = argv[1];

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

#ifdef _WIN32
	// Reference: http://sdl.beuc.net/sdl.wiki/FAQ_Console
	// TODO: Set console as UTF-8.
	freopen("CON", "w", stdout);
	freopen("CON", "w", stderr);
#endif /* _WIN32 */

	// Initialize LibGens.
	LibGens::Init();

	// Register the LibGens OSD handler.
	lg_set_osd_fn(GensSdl::gsdl_osd);

	// Load the ROM image.
	rom = new Rom(rom_filename);
	if (!rom->isOpen()) {
		// Error opening the ROM.
		// TODO: Error code?
		fprintf(stderr, "Error opening ROM file %s: (TODO get error code)\n",
			rom_filename);
		return EXIT_FAILURE;
	}
	if (rom->isMultiFile()) {
		// Select the first file.
		rom->select_z_entry(rom->get_z_entry_list());
	}

	// Set the SRAM/EEPROM path.
	LibGens::EmuContext::SetPathSRam(GensSdl::getConfigDir("SRAM").c_str());

	// Create the emulation context.
	context = new EmuMD(rom);
	if (!context->isRomOpened()) {
		// Error loading the ROM into EmuMD.
		// TODO: Error code?
		fprintf(stderr, "Error initializing EmuContext for %s: (TODO get error code)\n",
			rom_filename);
		return EXIT_FAILURE;
	}

	// Initialize SDL handlers.
	sdlHandler = new SdlHandler();
	if (sdlHandler->init_video() < 0)
		return EXIT_FAILURE;
	if (sdlHandler->init_audio() < 0)
		return EXIT_FAILURE;

	// Start the frame timer.
	// TODO: Region code?
	LibGens::Timing timing;
	bool isPal = false;
	const unsigned int usec_per_frame = (1000000 / (isPal ? 50 : 60));
	uint64_t start_clk = timing.getTime();
	uint64_t old_clk = start_clk;
	uint64_t fps_clk = start_clk;
	uint64_t new_clk = start_clk;
	// Microsecond counter for frameskip.
	uint64_t usec_frameskip = 0;

	// Frame counters.
	unsigned int frames = 0;
	unsigned int frames_old = 0;
	unsigned int fps = 0;	// TODO: float or double?

	// Enable frameskip.
	bool frameskip = true;

	// TODO: Close the ROM, or let EmuContext do it?

	// Set the color depth.
	MdFb *fb = context->m_vdp->MD_Screen->ref();
	fb->setBpp(MdFb::BPP_32);

	// Set the SDL video source.
	sdlHandler->set_video_source(fb);

	// Set the window title.
	SDL_WM_SetCaption("Gens/GS II [SDL]", nullptr);

	// Start audio.
	SDL_PauseAudio(0);

	// Initialize the I/O Manager with a default key layout.
	keyManager = new KeyManager();
	keyManager->setIoType(IoManager::VIRTPORT_1, IoManager::IOT_6BTN);
	keyManager->setKeyMap(IoManager::VIRTPORT_1, keyMap, ARRAY_SIZE(keyMap));
	keyManager->setIoType(IoManager::VIRTPORT_2, IoManager::IOT_NONE);

	bool running = true;
	bool paused = false;
	while (running) {
		SDL_Event event;
		int ret = (paused
			? SDL_WaitEvent(&event)
			: SDL_PollEvent(&event));
		if (ret) {
			switch (event.type) {
				case SDL_QUIT:
					running = 0;
					break;

				case SDL_KEYDOWN:
					// SDL keycodes nearly match GensKey.
					// TODO: Split out into a separate function?
					switch (event.key.keysym.sym) {
						case SDLK_TAB:
							// Check for Shift.
							if (event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) {
								// Hard Reset.
								context->hardReset();
							} else {
								// Soft Reset.
								context->softReset();
							}
							break;
						case SDLK_ESCAPE:
							// Pause emulation.
							// TODO: Apply the pause effect.
							paused = !paused;
							// Reset the clocks and counters.
							frames_old = frames;
							fps = 0;
							start_clk = timing.getTime();
							old_clk = start_clk;
							fps_clk = start_clk;
							new_clk = start_clk;
							// Pause audio.
							SDL_PauseAudio(paused);
							// Autosave SRAM/EEPROM.
							context->autoSaveData(-1);
							// TODO: Reset the audio ringbuffer?
							// Update the window title.
							if (paused) {
								SDL_WM_SetCaption("Gens/GS II [SDL] [Paused]", nullptr);
							} else {
								SDL_WM_SetCaption("Gens/GS II [SDL]", nullptr);
							}
							break;
						case SDLK_BACKSPACE:
							if (event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) {
								// Take a screenshot.
								GensSdl::doScreenShot(fb, context->m_vdp, rom->filenameBaseNoExt().c_str());
							}
							break;
						default:
							// Send the key to the KeyManager.
							keyManager->keyDown(event.key.keysym.sym);
							break;
					}
					break;

				case SDL_KEYUP:
					// SDL keycodes nearly match GensKey.
					keyManager->keyUp(event.key.keysym.sym);
					break;

				default:
					break;
			}
		}

		if (paused) {
			// Don't do anything.
			continue;
		}

		// New start time.
		new_clk = timing.getTime();

		// Update the FPS counter.
		unsigned int fps_tmp = ((new_clk - fps_clk) & 0x3FFFFF);
		if (fps_tmp >= 1000000) {
			// More than 1 second has passed.
			fps_clk = new_clk;
			if (frames_old > frames) {
				fps = (frames_old - frames);
			} else {
				fps = (frames - frames_old);
			}
			frames_old = frames;

			// Update the window title.
			// TODO: Average the FPS over multiple seconds
			// and/or quarter-seconds.
			char win_title[256];
			snprintf(win_title, sizeof(win_title), "Gens/GS II [SDL] - %d fps", fps);
			SDL_WM_SetCaption(win_title, nullptr);
		}

		// Frameskip.
		if (frameskip) {
			// Determine how many frames to run.
			usec_frameskip += ((new_clk - old_clk) & 0x3FFFFF); // no more than 4 secs
			unsigned int frames_todo = (unsigned int)(usec_frameskip / usec_per_frame);
			usec_frameskip %= usec_per_frame;
			old_clk = new_clk;

			if (frames_todo == 0) {
				// No frames to do yet.
				// Wait until the next frame.
				uint64_t usec_sleep = (usec_per_frame - usec_frameskip);
				if (usec_sleep > 1000) {
					// Never sleep for longer than the 50 Hz value
					// so events are checked often enough.
					if (usec_sleep > (1000000 / 50)) {
						usec_sleep = (1000000 / 50);
					}
					usec_sleep -= 1000;

#ifdef _WIN32
					// Win32: Use a yield() loop.
					// FIXME: Doesn't work properly on VBox/WinXP...
					uint64_t yield_end = timing.getTime() + usec_sleep;
					do {
						yield();
					} while (yield_end > timing.getTime());
#else /* !_WIN32 */
					// Linux: Use usleep().
					usleep(usec_sleep);
#endif /* _WIN32 */
				}
			} else {
				// Draw frames.
				for (; frames_todo != 1; frames_todo--) {
					// Run a frame without rendering.
					context->execFrameFast();
					sdlHandler->update_audio();
				}
				frames_todo = 0;

				// Run a frame and render it.
				context->execFrame();
				sdlHandler->update_audio();
				sdlHandler->update_video();
				// Increment the frame counter.
				frames++;

				// Autosave SRAM/EEPROM.
				// TODO: EmuContext::execFrame() should probably do this itself...
				context->autoSaveData(1);
			}
		} else {
			// Run a frame and render it.
			context->execFrame();
			sdlHandler->update_audio();
			sdlHandler->update_video();
			// Increment the frame counter.
			frames++;

			// Autosave SRAM/EEPROM.
			// TODO: EmuContext::execFrame() should probably do this itself...
			context->autoSaveData(1);
		}

		// Update the I/O manager.
		keyManager->updateIoManager(context->m_ioManager);
	}

	// Save SRAM/EEPROM, if necessary.
	// TODO: Move to EmuContext::~EmuContext()?
	context->saveData();

	// Unregister the LibGens OSD handler.
	lg_set_osd_fn(nullptr);

	// NOTE: Deleting sdlHandler can cause crashes on Windows
	// due to the timer callback trying to post the semaphore
	// after it's been deleted.
	// Shut down the SDL functions manually.
	sdlHandler->end_audio();
	sdlHandler->end_video();
	//delete sdlHandler;

	// Shut. Down. EVERYTHING.
	delete keyManager;
	delete context;
	delete rom;
	fb->unref();
	return EXIT_SUCCESS;
}
