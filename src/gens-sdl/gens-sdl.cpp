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
#include "libgens/EmuContext/EmuContext.hpp"
#include "libgens/EmuContext/EmuContextFactory.hpp"
using LibGens::EmuContext;
using LibGens::EmuContextFactory;

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
#include <vector>
using std::string;
using std::vector;

#include <SDL.h>

namespace GensSdl {

static SdlHandler *sdlHandler = nullptr;
static Rom *rom = nullptr;
static EmuContext *context = nullptr;
static const char *rom_filename = nullptr;
static bool isPico = false;

static KeyManager *keyManager = nullptr;
// MD 6-button keyMap.
static const GensKey_t keyMap_md[] = {
	KEYV_UP, KEYV_DOWN, KEYV_LEFT, KEYV_RIGHT,	// UDLR
	KEYV_s, KEYV_d, KEYV_a, KEYV_RETURN,		// BCAS
	KEYV_e, KEYV_w, KEYV_q, KEYV_RSHIFT		// ZYXM
};
// Sega Pico keyMap.
static const GensKey_t keyMap_pico[] = {
	KEYV_UP, KEYV_DOWN, KEYV_LEFT, KEYV_RIGHT,		// UDLR
	KEYV_SPACE, KEYV_PAGEDOWN, KEYV_PAGEUP, KEYV_RETURN	// BCAS
	, 0, 0, 0, 0
};

// Startup OSD message queue.
struct OsdStartup {
	int duration;
	const char *msg;
	int param;
};
static vector<OsdStartup> startup_queue;

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
		if (sdlHandler) {
			sdlHandler->osd_printf(1500, msg, param);
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

static LibGens::Timing timing;

// Emulation state.
static bool running = true;
static bool paused = false;

// Enable frameskip.
static bool frameskip = true;

// Window has been exposed.
// Video should be updated if emulation is paused.
static bool exposed = false;

// Frameskip timers.
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
static clks_t clks;

// Save slot.
static int saveSlot_selected = 0;

/**
 * Save slot selection.
 * @param saveSlot Save slot. (0-9)
 */
static void doSaveSlot(int saveSlot)
{
	assert(saveSlot >= 0 && saveSlot <= 9);
	if (saveSlot < 0 || saveSlot > 9)
		return;
	saveSlot_selected = saveSlot;

	// Check if the specified savestate exists.
	// TODO: R_OK or just F_OK?
	const char *slot_state;
	string filename = getSavestateFilename(rom, saveSlot);
	if (!access(filename.c_str(), F_OK)) {
		// Savestate exists.
		slot_state = "OCCUPIED";
		// TODO: Load the preview image.
	} else {
		// Savestate does not exist.
		slot_state = "EMPTY";
	}

	// Show an OSD message.
	sdlHandler->osd_printf(1500, "Save Slot %d [%s]", saveSlot, slot_state);
}

/**
 * Load the state in the selected slot.
 */
static void doLoadState(void)
{
	assert(saveSlot_selected >= 0 && saveSlot_selected <= 9);
	if (saveSlot_selected < 0 || saveSlot_selected > 9)
		return;

	string filename = getSavestateFilename(rom, saveSlot_selected);
	int ret = context->zomgLoad(filename.c_str());
	if (ret == 0) {
		// State loaded.
		sdlHandler->osd_printf(1500, "Save Slot %d loaded.", saveSlot_selected);
	} else {
		// Error loading state.
		if (ret == -ENOENT) {
			// File not found.
			sdlHandler->osd_printf(1500, "Save Slot %d is empty.", saveSlot_selected);
		} else {
			// Other error.
			sdlHandler->osd_printf(1500,
				"Error loading Save Slot %d:\n* %s",
				saveSlot_selected, strerror(-ret));
		}
	}
}

/**
 * Save the state in the selected slot.
 */
static void doSaveState(void)
{
	assert(saveSlot_selected >= 0 && saveSlot_selected <= 9);
	if (saveSlot_selected < 0 || saveSlot_selected > 9)
		return;

	string filename = getSavestateFilename(rom, saveSlot_selected);
	int ret = context->zomgSave(filename.c_str());
	if (ret == 0) {
		// State saved.
		sdlHandler->osd_printf(1500, "Save Slot %d saved.", saveSlot_selected);
	} else {
		// Error saving state.
		sdlHandler->osd_printf(1500,
				"Error saving Save Slot %d:\n* %s",
				saveSlot_selected, strerror(-ret));
	}
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
						sdlHandler->osd_printf(1500, "Hard Reset.");
					} else {
						// Soft Reset.
						context->softReset();
						sdlHandler->osd_printf(1500, "Soft Reset.");
					}
					break;

				case SDLK_ESCAPE:
					// Pause emulation.
					// TODO: Apply the pause effect.
					paused = !paused;
					// Reset the clocks and counters.
					clks.reset();
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
						GensSdl::doScreenShot(context->m_vdp->MD_Screen, rom);
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

				case SDLK_0: case SDLK_1:
				case SDLK_2: case SDLK_3:
				case SDLK_4: case SDLK_5:
				case SDLK_6: case SDLK_7:
				case SDLK_8: case SDLK_9:
					// Save slot selection.
					doSaveSlot(event.key.keysym.sym - SDLK_0);
					break;

				case SDLK_F5:
					// Save state.
					doSaveState();
					break;

				case SDLK_F6: {
					// Previous save slot.
					int saveSlot  = ((saveSlot_selected + 9) % 10);
					doSaveSlot(saveSlot);
					break;
				}

				case SDLK_F7: {
					// Next save slot.
					int saveSlot  = ((saveSlot_selected + 1) % 10);
					doSaveSlot(saveSlot);
					break;
				}

				case SDLK_F8:
					// Load state.
					doLoadState();
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
				case SDL_WINDOWEVENT_EXPOSED:
					// Window has been exposed.
					// Tell the main loop to update video.
					exposed = true;
					break;
			}
			break;

		default:
			break;
	}
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

	// Is the ROM format supported?
	if (!EmuContextFactory::isRomFormatSupported(rom)) {
		// ROM format is not supported.
		const char *rom_format = romFormatToString(rom->romFormat());
		fprintf(stderr, "Error loading ROM file %s: ROM is in %s format.\nOnly plain binary and SMD-format ROMs are supported.\n",
			rom_filename, rom_format);
		return EXIT_FAILURE;
	}

	// Check the ROM's system ID.
	if (!EmuContextFactory::isRomSystemSupported(rom)) {
		// System is not supported.
		const char *rom_sysId = sysIdToString(rom->sysId());
		fprintf(stderr, "Error loading ROM file %s: ROM is for %s.\nOnly Mega Drive and Pico ROMs are supported.\n",
			rom_filename, rom_sysId);
		return EXIT_FAILURE;
	}

	// Check for Pico controller.
	isPico = false;
	if (rom->sysId() == Rom::MDP_SYSTEM_PICO) {
		isPico = true;
	}

	// Set the SRAM/EEPROM path.
	EmuContext::SetPathSRam(getConfigDir("SRAM").c_str());

	// Create the emulation context.
	context = EmuContextFactory::createContext(rom);
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

	// Set the window title.
	sdlHandler->set_window_title("Gens/GS II [SDL]");

	// Check for startup messages.
	if (!startup_queue.empty()) {
		for (int i = 0; i < (int)startup_queue.size(); i++) {
			const OsdStartup &startup = startup_queue.at(i);
			sdlHandler->osd_printf(startup.duration, startup.msg, startup.param);
		}
	}

	// Start the frame timer.
	// TODO: Region code?
	bool isPal = false;
	const unsigned int usec_per_frame = (1000000 / (isPal ? 50 : 60));
	clks.reset();

	// Enable frameskip.
	frameskip = true;

	// TODO: Close the ROM, or let EmuContext do it?

	// Set the color depth.
	MdFb *fb = context->m_vdp->MD_Screen->ref();
	fb->setBpp(MdFb::BPP_32);

	// Set the SDL video source.
	sdlHandler->set_video_source(fb);

	// Start audio.
	sdlHandler->pause_audio(false);

	// Initialize the I/O Manager with a default key layout.
	keyManager = new KeyManager();
	if (!isPico) {
		// Standard Mega Drive controllers.
		keyManager->setIoType(IoManager::VIRTPORT_1, IoManager::IOT_6BTN);
		keyManager->setKeyMap(IoManager::VIRTPORT_1, keyMap_md, ARRAY_SIZE(keyMap_md));
		keyManager->setIoType(IoManager::VIRTPORT_2, IoManager::IOT_NONE);
	} else {
		// Sega Pico controller.
		keyManager->setIoType(IoManager::VIRTPORT_1, IoManager::IOT_PICO);
		keyManager->setKeyMap(IoManager::VIRTPORT_1, keyMap_pico, ARRAY_SIZE(keyMap_pico));
		keyManager->setIoType(IoManager::VIRTPORT_2, IoManager::IOT_NONE);
	}

	while (running) {
		SDL_Event event;
		int ret;
		if (paused) {
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
			processSdlEvent(event);
		}

		if (paused) {
			// Emulation is paused.
			// Only update video if the VBackend is dirty
			// or the SDL window has been exposed.
			if (exposed) {
				sdlHandler->update_video();
			} else {
				sdlHandler->update_video_paused();
			}

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

		// Frameskip.
		if (frameskip) {
			// Determine how many frames to run.
			clks.usec_frameskip += ((clks.new_clk - clks.old_clk) & 0x3FFFFF); // no more than 4 secs
			unsigned int frames_todo = (unsigned int)(clks.usec_frameskip / usec_per_frame);
			clks.usec_frameskip %= usec_per_frame;
			clks.old_clk = clks.new_clk;

			if (frames_todo == 0) {
				// No frames to do yet.
				// Wait until the next frame.
				uint64_t usec_sleep = (usec_per_frame - clks.usec_frameskip);
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
				clks.frames++;

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
			clks.frames++;

			// Autosave SRAM/EEPROM.
			// TODO: EmuContext::execFrame() should probably do this itself...
			context->autoSaveData(1);
		}

		// Update the I/O manager.
		keyManager->updateIoManager(context->m_ioManager);
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
