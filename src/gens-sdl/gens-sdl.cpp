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
#include "VBackend.hpp"
using GensSdl::SdlHandler;
using GensSdl::VBackend;

#include "str_lookup.hpp"

// LibGens
#include "libgens/lg_main.hpp"
#include "libgens/lg_osd.h"
#include "libgens/Rom.hpp"
#include "libgens/Util/MdFb.hpp"
#include "libgens/Util/Timing.hpp"
using LibGens::Rom;
using LibGens::MdFb;
using LibGens::Timing;

// Emulation Context.
#include "libgens/EmuContext/EmuMD.hpp"
#include "libgens/EmuContext/EmuPico.hpp"
using LibGens::EmuContext;
using LibGens::EmuMD;
using LibGens::EmuPico;

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

// TODO: Move to GensSdl?
static SdlHandler *sdlHandler = nullptr;
static Rom *rom = nullptr;
static EmuContext *context = nullptr;
static const char *rom_filename = nullptr;
static bool isPico = false;
static bool picoWasPrevPressed = false;
static bool picoWasNextPressed = false;

static KeyManager *keyManager = nullptr;
static const GensKey_t keyMap[] = {
	KEYV_UP, KEYV_DOWN, KEYV_LEFT, KEYV_RIGHT,	// UDLR
	KEYV_s, KEYV_d, KEYV_a, KEYV_RETURN,		// BCAS
	KEYV_e, KEYV_w, KEYV_q, KEYV_RSHIFT		// ZYXM
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

static LibGens::Timing timing;

// Emulation state.
static bool running = true;
static bool paused = false;

// Enable frameskip.
static bool frameskip = true;

// Frameskip timers.
static uint64_t start_clk;
static uint64_t old_clk;
static uint64_t fps_clk;
static uint64_t new_clk;
// Microsecond counter for frameskip.
static uint64_t usec_frameskip;

// Frame counters.
unsigned int frames = 0;
unsigned int frames_old = 0;
unsigned int fps = 0;	// TODO: float or double?

// Reset frameskip timers.
static void reset_frameskip_timers(void) {
	start_clk = timing.getTime();
	old_clk = start_clk;
	fps_clk = start_clk;
	fps_clk = start_clk;
	new_clk = start_clk;
	usec_frameskip = 0;

	// Frame counter.
	frames_old = frames;
	fps = 0;
}

/**
 * Process an SDL event.
 * @param event SDL event.
 */
static void processSdlEvent(const SDL_Event &event) {
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
					GensSdl::reset_frameskip_timers();
					// Pause audio.
					sdlHandler->pause_audio(paused);
					// Autosave SRAM/EEPROM.
					context->autoSaveData(-1);
					// TODO: Reset the audio ringbuffer?
					// Update the window title.
					if (paused) {
						sdlHandler->set_window_title("Gens/GS II [SDL] [Paused]");
					} else {
						sdlHandler->set_window_title("Gens/GS II [SDL]");
					}
					break;

				case SDLK_BACKSPACE:
					if (event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) {
						// Take a screenshot.
						GensSdl::doScreenShot(context->m_vdp->MD_Screen,
							rom->filenameBaseNoExt().c_str());
					}
					break;

				case SDLK_F2:
					if (event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) {
						// Change stretch mode parameters.
						// TODO: OSD message, but only if the backend supports it?
						int stretchMode = (int)sdlHandler->vBackend()->stretchMode();
						stretchMode++;
						stretchMode &= 3;
						sdlHandler->vBackend()->setStretchMode((VBackend::StretchMode_t)stretchMode);
						// TODO: Update video if paused.
					}
					break;

				case SDLK_RETURN:
					// Check for Alt+Enter.
					if ((event.key.keysym.mod & KMOD_ALT) &&
					    !(event.key.keysym.mod & ~KMOD_ALT))
					{
						// Alt+Enter. Toggle fullscreen.
						sdlHandler->toggle_fullscreen();
					} else {
						// Not Alt+Enter.
						// Send the key to the KeyManager.
						keyManager->keyDown(SdlHandler::scancodeToGensKey(event.key.keysym.scancode));
					}
					break;

				default:
					// Send the key to the KeyManager.
					keyManager->keyDown(SdlHandler::scancodeToGensKey(event.key.keysym.scancode));
					break;
			}
			break;

		case SDL_KEYUP:
			// SDL keycodes nearly match GensKey.
			keyManager->keyUp(SdlHandler::scancodeToGensKey(event.key.keysym.scancode));
			break;

		case SDL_WINDOWEVENT:
			switch (event.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					// Resize the video renderer.
					sdlHandler->resize_video(event.window.data1, event.window.data2);
					break;
				default:
					break;
			}
			break;

		default:
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

	// Check the ROM format.
	switch (rom->romFormat()) {
		case Rom::RFMT_BINARY:
			// ROM format is supported.
			break;

		default:
			// ROM format is not supported.
			const char *rom_format = GensSdl::romFormatToString(rom->romFormat());
			fprintf(stderr, "Error loading ROM file %s: ROM is in %s format.\nOnly plain binary ROMs are supported.\n",
				rom_filename, rom_format);
			return EXIT_FAILURE;
	}

	// Check the ROM system.
	// TODO: Split into a separate function?
	switch (rom->sysId()) {
		case Rom::MDP_SYSTEM_MD:
			// System is supported.
			isPico = false;
			break;

		case Rom::MDP_SYSTEM_PICO:
			// System is supported.
			isPico = true;
			break;

		default:
			// System is not supported.
			const char *rom_sysId = GensSdl::sysIdToString(rom->sysId());
			fprintf(stderr, "Error loading ROM file %s: ROM is for %s.\nOnly Mega Drive ROMs are supported.\n",
				rom_filename, rom_sysId);
			return EXIT_FAILURE;
	}

	// Set the SRAM/EEPROM path.
	LibGens::EmuContext::SetPathSRam(GensSdl::getConfigDir("SRAM").c_str());

	// Create the emulation context.
	// TODO: Factory class that uses rom->sysId()?
	switch (rom->sysId()) {
		case Rom::MDP_SYSTEM_MD:
			context = new EmuMD(rom);
			break;
		case Rom::MDP_SYSTEM_PICO:
			context = new EmuPico(rom);
			break;
		default:
			context = nullptr;
			break;
	}
	if (!context || !context->isRomOpened()) {
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
	bool isPal = false;
	const unsigned int usec_per_frame = (1000000 / (isPal ? 50 : 60));
	GensSdl::reset_frameskip_timers();

	// Frame counters.
	unsigned int frames = 0;
	unsigned int frames_old = 0;
	unsigned int fps = 0;	// TODO: float or double?

	// Enable frameskip.
	GensSdl::frameskip = true;

	// TODO: Close the ROM, or let EmuContext do it?

	// Set the color depth.
	MdFb *fb = context->m_vdp->MD_Screen->ref();
	fb->setBpp(MdFb::BPP_32);

	// Set the SDL video source.
	sdlHandler->set_video_source(fb);

	// Set the window title.
	sdlHandler->set_window_title("Gens/GS II [SDL]");

	// Start audio.
	sdlHandler->pause_audio(false);

	// Initialize the I/O Manager with a default key layout.
	keyManager = new KeyManager();
	keyManager->setIoType(IoManager::VIRTPORT_1, IoManager::IOT_6BTN);
	keyManager->setKeyMap(IoManager::VIRTPORT_1, keyMap, ARRAY_SIZE(keyMap));
	keyManager->setIoType(IoManager::VIRTPORT_2, IoManager::IOT_NONE);

	while (GensSdl::running) {
		SDL_Event event;
		int ret;
		if (GensSdl::paused) {
			// Emulation is paused.
			// Wait for an SDL event.
			ret = SDL_WaitEvent(&event);
		} else {
			// Emulation is running.
			// Poll for an SDL event,
			// since we don't want to block frames.
			ret = SDL_PollEvent(&event);
		}

		if (ret) {
			// An SDL event has been received.
			// Process it.
			GensSdl::processSdlEvent(event);
		}

		if (GensSdl::paused) {
			// Emulation is paused.
			// Only update video if the VBackend is dirty.
			sdlHandler->update_video_paused();
			// Don't run any frames.
			continue;
		}

		// New start time.
		GensSdl::new_clk = GensSdl::timing.getTime();

		// Update the FPS counter.
		unsigned int fps_tmp = ((GensSdl::new_clk - GensSdl::fps_clk) & 0x3FFFFF);
		if (fps_tmp >= 1000000) {
			// More than 1 second has passed.
			GensSdl::fps_clk = GensSdl::new_clk;
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
			sdlHandler->set_window_title(win_title);
		}

		// Frameskip.
		if (GensSdl::frameskip) {
			// Determine how many frames to run.
			GensSdl::usec_frameskip += ((GensSdl::new_clk - GensSdl::old_clk) & 0x3FFFFF); // no more than 4 secs
			unsigned int frames_todo = (unsigned int)(GensSdl::usec_frameskip / usec_per_frame);
			GensSdl::usec_frameskip %= usec_per_frame;
			GensSdl::old_clk = GensSdl::new_clk;

			if (frames_todo == 0) {
				// No frames to do yet.
				// Wait until the next frame.
				uint64_t usec_sleep = (usec_per_frame - GensSdl::usec_frameskip);
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

		// Check for Pico page control buttons.
		// TODO: Create an IoPico controller instead of reusing Io3BTN?
		if (isPico) {
			// FIXME: We can't easily access the button bitfields.
			// Check the keystate manually.
			bool picoPrevPressed = keyManager->isKeyPressed(keyMap[6]);
			if (!picoWasPrevPressed && picoPrevPressed) {
				// Previous page.
				uint8_t pg = context->m_ioManager->picoPrevPage();
				printf("Pico: Page set to %u.\n", pg);
			}

			bool picoNextPressed = keyManager->isKeyPressed(keyMap[5]);
			if (!picoWasNextPressed && picoNextPressed) {
				// Next page.
				uint8_t pg = context->m_ioManager->picoNextPage();
				printf("Pico: Page set to %u.\n", pg);
			}

			picoWasPrevPressed = picoPrevPressed;
			picoWasNextPressed = picoNextPressed;
		}
	}

	// Pause audio and wait 50ms for SDL to catch up.
	sdlHandler->pause_audio(true);
	usleep(50000);

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
