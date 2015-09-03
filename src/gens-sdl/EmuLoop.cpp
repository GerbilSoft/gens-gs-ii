/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * EmuLoop.hpp: Main emulation loop.                                       *
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

#include "EmuLoop.hpp"
#include "gens-sdl.hpp"

// Configuration.
#include "Config.hpp"

#include "SdlHandler.hpp"
#include "VBackend.hpp"
using GensSdl::SdlHandler;
using GensSdl::VBackend;

// String lookup for ROM information.
#include "str_lookup.hpp"

// LibGens
#include "libgens/Rom.hpp"
#include "libgens/Util/MdFb.hpp"
using LibGens::Rom;
using LibGens::MdFb;

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

// LibZomg
#include "libzomg/Zomg.hpp"
#include "libzomg/img_data.h"
using LibZomg::ZomgBase;
using LibZomg::Zomg;

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

class EmuLoopPrivate
{
	public:
		EmuLoopPrivate();
		~EmuLoopPrivate();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		EmuLoopPrivate(const EmuLoopPrivate &);
		EmuLoopPrivate &operator=(const EmuLoopPrivate &);

	public:
		Rom *rom;
		bool isPico;
		bool frameskip;	// If true, enable frameskip.

		EmuContext *emuContext;
		KeyManager *keyManager;

		// Save slot.
		int saveSlot_selected;

		// Keymaps.
		static const GensKey_t keyMap_md[];
		static const GensKey_t keyMap_pico[];
};

/** EmuLoopPrivate **/

// MD 6-button keyMap.
const GensKey_t EmuLoopPrivate::keyMap_md[] = {
	KEYV_UP, KEYV_DOWN, KEYV_LEFT, KEYV_RIGHT,	// UDLR
	KEYV_s, KEYV_d, KEYV_a, KEYV_RETURN,		// BCAS
	KEYV_e, KEYV_w, KEYV_q, KEYV_RSHIFT		// ZYXM
};
// Sega Pico keyMap.
const GensKey_t EmuLoopPrivate::keyMap_pico[] = {
	KEYV_UP, KEYV_DOWN, KEYV_LEFT, KEYV_RIGHT,		// UDLR
	KEYV_SPACE, KEYV_PAGEDOWN, KEYV_PAGEUP, KEYV_RETURN	// BCAS
	, 0, 0, 0, 0
};

EmuLoopPrivate::EmuLoopPrivate()
	: rom(nullptr)
	, isPico(false)
	, frameskip(true)
	, emuContext(nullptr)
	, keyManager(nullptr)
	, saveSlot_selected(0)
{ }

EmuLoopPrivate::~EmuLoopPrivate()
{
	delete rom;
	delete emuContext;
	delete keyManager;
}

/** EmuLoop **/

EmuLoop::EmuLoop()
	: d(new EmuLoopPrivate())
{ }

EmuLoop::~EmuLoop()
{
	delete d;
}

/**
 * Process an SDL event.
 * @param event SDL event.
 * @return 0 if the event was handled; non-zero if it wasn't.
 */
int EmuLoop::processSdlEvent(const SDL_Event *event) {
	int ret = 0;
	switch (event->type) {
		case SDL_KEYDOWN:
			// SDL keycodes nearly match GensKey.
			// TODO: Split out into a separate function?
			// TODO: Check for "no modifiers" for some keys?
			switch (event->key.keysym.sym) {
				case SDLK_TAB:
					// Check for Shift.
					if (event->key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) {
						// Hard Reset.
						d->emuContext->hardReset();
						m_vBackend->osd_print(1500, "Hard Reset.");
					} else {
						// Soft Reset.
						d->emuContext->softReset();
						m_vBackend->osd_print(1500, "Soft Reset.");
					}
					break;

				case SDLK_BACKSPACE:
					if (event->key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) {
						// Take a screenshot.
						doScreenShot();
					}
					break;

				case SDLK_F2:
					// NOTE: This is handled here instead of in the generic
					// processSdlEvent_common() because stretch functionality
					// isn't used in CrazyEffectLoop.
					if (event->key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) {
						// Change stretch mode parameters.
						doStretchMode();
					} else {
						// Not handling this event.
						ret = 1;
					}
					break;

				case SDLK_0: case SDLK_1:
				case SDLK_2: case SDLK_3:
				case SDLK_4: case SDLK_5:
				case SDLK_6: case SDLK_7:
				case SDLK_8: case SDLK_9:
					// Save slot selection.
					doSaveSlot(event->key.keysym.sym - SDLK_0);
					break;

				case SDLK_F5:
					// Save state.
					doSaveState();
					break;

				case SDLK_F6: {
					// Previous save slot.
					int saveSlot  = ((d->saveSlot_selected + 9) % 10);
					doSaveSlot(saveSlot);
					break;
				}

				case SDLK_F7: {
					// Next save slot.
					int saveSlot  = ((d->saveSlot_selected + 1) % 10);
					doSaveSlot(saveSlot);
					break;
				}

				case SDLK_F8:
					// Load state.
					doLoadState();
					break;

				default: {
					// Check if the base class event handler will handle this.
					int ret = EventLoop::processSdlEvent(event);
					if (ret != 0) {
						// Not handled.
						// Send the key to the KeyManager.
						d->keyManager->keyDown(SdlHandler::scancodeToGensKey(event->key.keysym.scancode));
						break;
					}
					break;
				}
			}
			break;

		case SDL_KEYUP:
			// SDL keycodes nearly match GensKey.
			d->keyManager->keyUp(SdlHandler::scancodeToGensKey(event->key.keysym.scancode));
			break;

		default:
			// Event not handled.
			ret = 1;
			break;
	}

	if (ret != 0) {
		// Event wasn't handled.
		// Try the base class.
		ret = EventLoop::processSdlEvent(event);
	}
	return ret;
}

/**
 * Get the modification time string for the specified save file.
 * @param zomg Save file.
 * @return String contianing the mtime, or an error message if invalid.
 */
string EmuLoop::getSaveSlot_mtime(const ZomgBase *zomg)
{
	// TODO: This function can probably be optimized more...

	// Slot state.
	char slot_state[48];

	// Check the mtime.
	// TODO: mtime=0 is invalid.
	bool doFullTimestamp = false;
	time_t cur_time = time(nullptr);
	time_t zomg_mtime = zomg->mtime();

	// zomg_mtime is needed for printing.
	// TODO: Custom localtime_r() if system version isn't available?
	struct tm tm_zomg_mtime;
	if (!localtime_r(&zomg_mtime, &tm_zomg_mtime)) {
		// Error converting zomg_mtime.
		return "occupied";
	}

	if (zomg_mtime > cur_time) {
		// Savestate was modified in teh future!!1!
		// Either that, or localtime_r() failed.
		doFullTimestamp = true;
		goto convert;
	}

	// Check if the times are "close enough" to omit the date.
	struct tm tm_cur_time;
	if (!localtime_r(&cur_time, &tm_cur_time)) {
		// Error converting cur_time.
		doFullTimestamp = true;
		goto convert;
	}

	// Check if the times are within the same day.
	if (tm_cur_time.tm_yday != tm_zomg_mtime.tm_yday) {
		// Not the same day.
		// Are the times within 12 hours?
		if (cur_time - zomg_mtime >= (3600*12)) {
			// More than 12 hours.
			// Show the full date.
			doFullTimestamp = true;
		}
	}

convert:
	if (doFullTimestamp) {
		// Show the full timestamp.
		strftime(slot_state, sizeof(slot_state), "%x %X", &tm_zomg_mtime);
	} else {
		// Show only the time.
		strftime(slot_state, sizeof(slot_state), "%X", &tm_zomg_mtime);
	}
	return string(slot_state);
}

/**
 * Save slot selection.
 * @param saveSlot Save slot. (0-9)
 */
void EmuLoop::doSaveSlot(int saveSlot)
{
	assert(saveSlot >= 0 && saveSlot <= 9);
	if (saveSlot < 0 || saveSlot > 9)
		return;
	d->saveSlot_selected = saveSlot;

	// Metadata variables.
	string slot_state;
	Zomg_Img_Data_t img_data;
	img_data.data = nullptr;

	// Check if the specified savestate exists.
	// TODO: R_OK or just F_OK?
	string filename = getSavestateFilename(d->rom, saveSlot);
	if (!access(filename.c_str(), F_OK)) {
		// Savestate exists.
		// Load some file information.
		LibZomg::Zomg zomg(filename.c_str(), Zomg::ZOMG_LOAD);
		if (!zomg.isOpen()) {
			// Error opening the savestate.
			slot_state = "error";
		} else {
			// Get the slot mtime.
			slot_state = getSaveSlot_mtime(&zomg);

			// Get the preview image.
			// TODO: Rename loadPreview() to loadPreviewImage()?
			int ret = zomg.loadPreview(&img_data);
			if (ret != 0) {
				// Image load failed.
				// TODO: Ensure all reading functions
				// called by loadPreview() free the
				// memory on error.
				img_data.data = nullptr;
			}
		}
	} else {
		// Savestate does not exist.
		slot_state = "empty";
	}

	// Show an OSD message.
	m_vBackend->osd_printf(1500, "Slot %d [%s]", saveSlot, slot_state.c_str());
	// If img_data.data is nullptr, this will hide the current image.
	m_vBackend->osd_preview_image(1500, &img_data);
	free(img_data.data);
}

/**
 * Load the state in the selected slot.
 */
void EmuLoop::doLoadState(void)
{
	assert(d->saveSlot_selected >= 0 && d->saveSlot_selected <= 9);
	if (d->saveSlot_selected < 0 || d->saveSlot_selected > 9)
		return;

	string filename = getSavestateFilename(d->rom, d->saveSlot_selected);
	int ret = d->emuContext->zomgLoad(filename.c_str());
	if (ret == 0) {
		// State loaded.
		m_vBackend->osd_printf(1500, "Slot %d loaded.", d->saveSlot_selected);
	} else {
		// Error loading state.
		if (ret == -ENOENT) {
			// File not found.
			m_vBackend->osd_printf(1500,
				"Slot %d is empty.",
				d->saveSlot_selected);
		} else {
			// Other error.
			m_vBackend->osd_printf(1500,
				"Error loading Slot %d:\n* %s",
				d->saveSlot_selected, strerror(-ret));
		}
	}
}

/**
 * Save the state in the selected slot.
 */
void EmuLoop::doSaveState(void)
{
	assert(d->saveSlot_selected >= 0 && d->saveSlot_selected <= 9);
	if (d->saveSlot_selected < 0 || d->saveSlot_selected > 9)
		return;

	string filename = getSavestateFilename(d->rom, d->saveSlot_selected);
	int ret = d->emuContext->zomgSave(filename.c_str());
	if (ret == 0) {
		// State saved.
		m_vBackend->osd_printf(1500,
				"Slot %d saved.",
				d->saveSlot_selected);
	} else {
		// Error saving state.
		m_vBackend->osd_printf(1500,
				"Error saving Slot %d:\n* %s",
				d->saveSlot_selected, strerror(-ret));
	}
}

/**
 * Change stretch mode parameters.
 */
void EmuLoop::doStretchMode(void)
{
	// Change stretch mode parameters.
	int stretchMode = (int)m_vBackend->stretchMode();
	stretchMode++;
	stretchMode &= 3;
	m_vBackend->setStretchMode((VBackend::StretchMode_t)stretchMode);

	// Show an OSD message.
	const char *stretch;
	switch (stretchMode) {
		case VBackend::STRETCH_NONE:
		default:
			stretch = "None";
			break;
		case VBackend::STRETCH_H:
			stretch = "Horizontal";
			break;
		case VBackend::STRETCH_V:
			stretch = "Vertical";
			break;
		case VBackend::STRETCH_FULL:
			stretch = "Full";
			break;
	}

	m_vBackend->osd_printf(1500, "Stretch Mode set to %s.", stretch);
}

/**
 * Take a screenshot.
 */
void EmuLoop::doScreenShot(void)
{
	int ret = GensSdl::doScreenShot(d->emuContext->m_vdp->MD_Screen, d->rom);
	if (ret >= 0) {
		m_vBackend->osd_printf(1500,
			"Screenshot %d saved.", ret);
	} else {
		m_vBackend->osd_printf(1500,
			"Error saving screenshot:\n* %s", strerror(-ret));
	}
}

/**
 * Run the event loop.
 * @param rom_filename ROM filename. [TODO: Replace with options struct?]
 * @return Exit code.
 */
int EmuLoop::run(const char *rom_filename)
{
	// Load the ROM image.
	d->rom = new Rom(rom_filename);
	if (!d->rom->isOpen()) {
		// Error opening the ROM.
		// TODO: Error code?
		fprintf(stderr, "Error opening ROM file %s: (TODO get error code)\n",
			rom_filename);
		return EXIT_FAILURE;
	}
	if (d->rom->isMultiFile()) {
		// Select the first file.
		d->rom->select_z_entry(d->rom->get_z_entry_list());
	}

	// Is the ROM format supported?
	if (!EmuContextFactory::isRomFormatSupported(d->rom)) {
		// ROM format is not supported.
		const char *rom_format = romFormatToString(d->rom->romFormat());
		fprintf(stderr, "Error loading ROM file %s: ROM is in %s format.\nOnly plain binary and SMD-format ROMs are supported.\n",
			rom_filename, rom_format);
		return EXIT_FAILURE;
	}

	// Check the ROM's system ID.
	if (!EmuContextFactory::isRomSystemSupported(d->rom)) {
		// System is not supported.
		const char *rom_sysId = sysIdToString(d->rom->sysId());
		fprintf(stderr, "Error loading ROM file %s: ROM is for %s.\nOnly Mega Drive and Pico ROMs are supported.\n",
			rom_filename, rom_sysId);
		return EXIT_FAILURE;
	}

	// Check for Pico controller.
	d->isPico = false;
	if (d->rom->sysId() == Rom::MDP_SYSTEM_PICO) {
		d->isPico = true;
	}

	// Set the SRAM/EEPROM path.
	EmuContext::SetPathSRam(getConfigDir("SRAM").c_str());

	// Create the emulation context.
	d->emuContext = EmuContextFactory::createContext(d->rom);
	if (!d->emuContext || !d->emuContext->isRomOpened()) {
		// Error loading the ROM into EmuMD.
		// TODO: Error code?
		fprintf(stderr, "Error initializing EmuContext for %s: (TODO get error code)\n",
			rom_filename);
		return EXIT_FAILURE;
	}

	// Initialize the SDL handlers.
	m_sdlHandler = new SdlHandler();
	if (m_sdlHandler->init_video() < 0)
		return EXIT_FAILURE;
	if (m_sdlHandler->init_audio() < 0)
		return EXIT_FAILURE;
	m_vBackend = m_sdlHandler->vBackend();

	// Set the window title.
	m_sdlHandler->set_window_title("Gens/GS II [SDL]");

	// Check for startup messages.
	checkForStartupMessages();

	// Start the frame timer.
	// TODO: Region code?
	bool isPal = false;
	const unsigned int usec_per_frame = (1000000 / (isPal ? 50 : 60));
	m_clks.reset();

	// TODO: Close the ROM, or let EmuContext do it?

	// Set the color depth.
	// TODO: Command line option?
	MdFb *fb = d->emuContext->m_vdp->MD_Screen->ref();
	fb->setBpp(MdFb::BPP_32);

	// Set the SDL video source.
	m_sdlHandler->set_video_source(fb);

	// Start audio.
	m_sdlHandler->pause_audio(false);

	// Initialize the I/O Manager with a default key layout.
	d->keyManager = new KeyManager();
	if (!d->isPico) {
		// Standard Mega Drive controllers.
		d->keyManager->setIoType(IoManager::VIRTPORT_1, IoManager::IOT_6BTN);
		d->keyManager->setKeyMap(IoManager::VIRTPORT_1, d->keyMap_md, ARRAY_SIZE(d->keyMap_md));
		d->keyManager->setIoType(IoManager::VIRTPORT_2, IoManager::IOT_NONE);
	} else {
		// Sega Pico controller.
		d->keyManager->setIoType(IoManager::VIRTPORT_1, IoManager::IOT_PICO);
		d->keyManager->setKeyMap(IoManager::VIRTPORT_1, d->keyMap_pico, ARRAY_SIZE(d->keyMap_pico));
		d->keyManager->setIoType(IoManager::VIRTPORT_2, IoManager::IOT_NONE);
	}

	// TODO: Move some more common stuff back to gens-sdl.cpp.
	uint8_t old_paused = 0;
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

		// Check if the 'paused' state was changed.
		// If it was, autosave SRAM/EEPROM.
		if (old_paused != m_paused.data) {
			// 'paused' state was changed.
			// (TODO: Only if paused == true?)
			d->emuContext->autoSaveData(-1);
			old_paused = m_paused.data;
		}

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

		// Frameskip.
		if (m_frameskip) {
			// Determine how many frames to run.
			m_clks.usec_frameskip += ((m_clks.new_clk - m_clks.old_clk) & 0x3FFFFF); // no more than 4 secs
			unsigned int frames_todo = (unsigned int)(m_clks.usec_frameskip / usec_per_frame);
			m_clks.usec_frameskip %= usec_per_frame;
			m_clks.old_clk = m_clks.new_clk;

			if (frames_todo == 0) {
				// No frames to do yet.
				// Wait until the next frame.
				uint64_t usec_sleep = (usec_per_frame - m_clks.usec_frameskip);
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
					uint64_t yield_end = m_clks.timing.getTime() + usec_sleep;
					do {
						yield();
					} while (yield_end > m_clks.timing.getTime());
#else /* !_WIN32 */
					// Linux: Use usleep().
					usleep(usec_sleep);
#endif /* _WIN32 */
				}
			} else {
				// Draw frames.
				for (; frames_todo != 1; frames_todo--) {
					// Run a frame without rendering.
					d->emuContext->execFrameFast();
					m_sdlHandler->update_audio();
				}
				frames_todo = 0;

				// Run a frame and render it.
				d->emuContext->execFrame();
				m_sdlHandler->update_audio();
				m_sdlHandler->update_video();
				// Increment the frame counter.
				m_clks.frames++;

				// Autosave SRAM/EEPROM.
				// TODO: EmuContext::execFrame() should probably do this itself...
				d->emuContext->autoSaveData(1);
			}
		} else {
			// Run a frame and render it.
			d->emuContext->execFrame();
			m_sdlHandler->update_audio();
			m_sdlHandler->update_video();
			// Increment the frame counter.
			m_clks.frames++;

			// Autosave SRAM/EEPROM.
			// TODO: EmuContext::execFrame() should probably do this itself...
			d->emuContext->autoSaveData(1);
		}

		// Update the I/O manager.
		d->keyManager->updateIoManager(d->emuContext->m_ioManager);
	}

	// Unreference the framebuffer.
	fb->unref();

	// Save SRAM/EEPROM, if necessary.
	// TODO: Move to EmuContext::~EmuContext()?
	d->emuContext->saveData();

	// Shut down LibGens.
	delete d->keyManager;
	d->keyManager = nullptr;
	delete d->emuContext;
	d->emuContext = nullptr;
	delete d->rom;
	d->rom = nullptr;

	// Pause audio and wait 50ms for SDL to catch up.
	m_sdlHandler->pause_audio(true);
	usleep(50000);

	// NOTE: Deleting sdlHandler can cause crashes on Windows
	// due to the timer callback trying to post the semaphore
	// after it's been deleted.
	// Shut down the SDL functions manually.
	m_sdlHandler->end_audio();
	m_sdlHandler->end_video();
	m_vBackend = nullptr;

	// Done running the emulation loop.
	return 0;
}

}
