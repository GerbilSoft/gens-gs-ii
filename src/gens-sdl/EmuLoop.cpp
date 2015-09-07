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
#include "libgens/Vdp/Vdp.hpp"
using LibGens::Rom;
using LibGens::MdFb;
using LibGens::Vdp;

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

// Command line parameters.
#include "Options.hpp"

// C++ includes.
#include <string>
using std::string;

#include "EventLoop_p.hpp"
namespace GensSdl {

class EmuLoopPrivate : public EventLoopPrivate
{
	public:
		EmuLoopPrivate();
		virtual ~EmuLoopPrivate();

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

		// Last 'paused' value.
		// Used to determine if SRAM/EEPROM
		// should be autosaved.
		paused_t last_paused;

		/**
		 * Get the modification time string for the specified save file.
		 * @param zomg Save file.
		 * @return String contianing the mtime, or an error message if invalid.
		 */
		static std::string getSaveSlot_mtime(const LibZomg::ZomgBase *zomg);

		/**
		 * Save slot selection.
		 * @param saveSlot Save slot. (0-9)
		 */
		void doSaveSlot(int saveSlot);

		/**
		 * Load the state in the selected slot.
		 */
		void doLoadState(void);

		/**
		 * Save the state in the selected slot.
		 */
		void doSaveState(void);

		/**
		 * Change stretch mode parameters.
		 */
		void doStretchMode(void);

		/**
		 * Take a screenshot.
		 */
		void doScreenShot(void);
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
	: EventLoopPrivate()
	, rom(nullptr)
	, isPico(false)
	, frameskip(true)
	, emuContext(nullptr)
	, keyManager(nullptr)
	, saveSlot_selected(0)
{
	last_paused.data = 0;
}

EmuLoopPrivate::~EmuLoopPrivate()
{
	delete rom;
	delete emuContext;
	delete keyManager;
}

/**
 * Get the modification time string for the specified save file.
 * @param zomg Save file.
 * @return String contianing the mtime, or an error message if invalid.
 */
string EmuLoopPrivate::getSaveSlot_mtime(const ZomgBase *zomg)
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
void EmuLoopPrivate::doSaveSlot(int saveSlot)
{
	assert(saveSlot >= 0 && saveSlot <= 9);
	if (saveSlot < 0 || saveSlot > 9)
		return;
	saveSlot_selected = saveSlot;

	// Metadata variables.
	string slot_state;
	Zomg_Img_Data_t img_data;
	img_data.data = nullptr;

	// Check if the specified savestate exists.
	// TODO: R_OK or just F_OK?
	string filename = getSavestateFilename(rom, saveSlot);
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
	vBackend->osd_printf(1500, "Slot %d [%s]", saveSlot, slot_state.c_str());
	// If img_data.data is nullptr, this will hide the current image.
	vBackend->osd_preview_image(1500, &img_data);
	free(img_data.data);
}

/**
 * Load the state in the selected slot.
 */
void EmuLoopPrivate::doLoadState(void)
{
	assert(saveSlot_selected >= 0 && saveSlot_selected <= 9);
	if (saveSlot_selected < 0 || saveSlot_selected > 9)
		return;

	string filename = getSavestateFilename(rom, saveSlot_selected);
	int ret = emuContext->zomgLoad(filename.c_str());
	if (ret == 0) {
		// State loaded.
		vBackend->osd_printf(1500, "Slot %d loaded.", saveSlot_selected);
	} else {
		// Error loading state.
		if (ret == -ENOENT) {
			// File not found.
			vBackend->osd_printf(1500,
				"Slot %d is empty.",
				saveSlot_selected);
		} else {
			// Other error.
			vBackend->osd_printf(1500,
				"Error loading Slot %d:\n* %s",
				saveSlot_selected, strerror(-ret));
		}
	}
}

/**
 * Save the state in the selected slot.
 */
void EmuLoopPrivate::doSaveState(void)
{
	assert(saveSlot_selected >= 0 && saveSlot_selected <= 9);
	if (saveSlot_selected < 0 || saveSlot_selected > 9)
		return;

	string filename = getSavestateFilename(rom, saveSlot_selected);
	int ret = emuContext->zomgSave(filename.c_str());
	if (ret == 0) {
		// State saved.
		vBackend->osd_printf(1500,
				"Slot %d saved.",
				saveSlot_selected);
	} else {
		// Error saving state.
		vBackend->osd_printf(1500,
				"Error saving Slot %d:\n* %s",
				saveSlot_selected, strerror(-ret));
	}
}

/**
 * Change stretch mode parameters.
 */
void EmuLoopPrivate::doStretchMode(void)
{
	// Change stretch mode parameters.
	int stretchMode = (int)vBackend->stretchMode();
	stretchMode++;
	stretchMode &= 3;
	vBackend->setStretchMode((VBackend::StretchMode_t)stretchMode);

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

	vBackend->osd_printf(1500, "Stretch Mode set to %s.", stretch);
}

/**
 * Take a screenshot.
 */
void EmuLoopPrivate::doScreenShot(void)
{
	int ret = GensSdl::doScreenShot(emuContext->m_vdp->MD_Screen, rom);
	if (ret >= 0) {
		vBackend->osd_printf(1500,
			"Screenshot %d saved.", ret);
	} else {
		vBackend->osd_printf(1500,
			"Error saving screenshot:\n* %s", strerror(-ret));
	}
}

/** EmuLoop **/

EmuLoop::EmuLoop()
	: EventLoop(new EmuLoopPrivate())
{ }

EmuLoop::~EmuLoop()
{ }

/**
 * Process an SDL event.
 * @param event SDL event.
 * @return 0 if the event was handled; non-zero if it wasn't.
 */
int EmuLoop::processSdlEvent(const SDL_Event *event) {
	EmuLoopPrivate *const d = d_func();
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
						d->vBackend->osd_print(1500, "Hard Reset.");
					} else {
						// Soft Reset.
						d->emuContext->softReset();
						d->vBackend->osd_print(1500, "Soft Reset.");
					}
					break;

				case SDLK_BACKSPACE:
					if (event->key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) {
						// Take a screenshot.
						d->doScreenShot();
					}
					break;

				case SDLK_F2:
					// NOTE: This is handled here instead of in the generic
					// processSdlEvent_common() because stretch functionality
					// isn't used in CrazyEffectLoop.
					if (event->key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) {
						// Change stretch mode parameters.
						d->doStretchMode();
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
					d->doSaveSlot(event->key.keysym.sym - SDLK_0);
					break;

				case SDLK_F5:
					// Save state.
					d->doSaveState();
					break;

				case SDLK_F6: {
					// Previous save slot.
					int saveSlot  = ((d->saveSlot_selected + 9) % 10);
					d->doSaveSlot(saveSlot);
					break;
				}

				case SDLK_F7: {
					// Next save slot.
					int saveSlot  = ((d->saveSlot_selected + 1) % 10);
					d->doSaveSlot(saveSlot);
					break;
				}

				case SDLK_F8:
					// Load state.
					d->doLoadState();
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
 * Run the event loop.
 * @param options Options.
 * @return Exit code.
 */
int EmuLoop::run(const Options *options)
{
	// Save options.
	// TODO: Make EmuLoop::run() non-virtual, save options there,
	// and then call protected virtual run_int()?
	EmuLoopPrivate *const d = d_func();
	d->options = options;
	
	// Load the ROM image.
	// NOTE: On gcc-5.x, if we store rom_filename().c_str(),
	// random corruption happens with filenames longer than
	// the short string buffer.
	string rom_filename = options->rom_filename();
	d->rom = new Rom(rom_filename.c_str());
	if (!d->rom->isOpen()) {
		// Error opening the ROM.
		// TODO: Error code?
		fprintf(stderr, "Error opening ROM file %s: (TODO get error code)\n",
			rom_filename.c_str());
		delete d->rom;
		d->rom = nullptr;
		return EXIT_FAILURE;
	}

	if (d->rom->isMultiFile()) {
		// Select the first file.
		d->rom->select_z_entry(d->rom->get_z_entry_list());
	}

	// Is a TMSS ROM filename specified?
	if (options->is_tmss_enabled()) {
		EmuContext::SetTmssRomFilename(options->tmss_rom_filename());
		EmuContext::SetTmssEnabled(true);
	}

	// Is the ROM format supported?
	if (!EmuContextFactory::isRomFormatSupported(d->rom)) {
		// ROM format is not supported.
		const char *rom_format = romFormatToString(d->rom->romFormat());
		fprintf(stderr, "Error loading ROM file %s: ROM is in %s format.\n"
			"Only plain binary and SMD-format ROMs are supported.\n",
			rom_filename.c_str(), rom_format);
		return EXIT_FAILURE;
	}

	// Check the ROM's system ID.
	if (!EmuContextFactory::isRomSystemSupported(d->rom)) {
		// System is not supported.
		const char *rom_sysId = sysIdToString(d->rom->sysId());
		fprintf(stderr, "Error loading ROM file %s: ROM is for %s.\n"
			"Only Mega Drive and Pico ROMs are supported.\n",
			rom_filename.c_str(), rom_sysId);
		return EXIT_FAILURE;
	}

	// Check for Pico controller.
	d->isPico = false;
	if (d->rom->sysId() == Rom::MDP_SYSTEM_PICO) {
		d->isPico = true;
	}

	// Set the SRAM/EEPROM path.
	EmuContext::SetPathSRam(getConfigDir("SRAM").c_str());

	// Set some static EmuContext properties.
	EmuContext::SetAutoFixChecksum(options->auto_fix_checksum());

	// Create the emulation context.
	d->emuContext = EmuContextFactory::createContext(d->rom);
	if (!d->emuContext || !d->emuContext->isRomOpened()) {
		// Error loading the ROM into EmuMD.
		// TODO: Error code?
		fprintf(stderr, "Error initializing EmuContext for %s: (TODO get error code)\n",
			rom_filename.c_str());
		return EXIT_FAILURE;
	}

	// Set VDP properties.
	// TODO: More properties?
	Vdp *vdp = d->emuContext->m_vdp;
	vdp->options.spriteLimits = options->sprite_limits();

	// Initialize the SDL handlers.
	d->sdlHandler = new SdlHandler();
	if (d->sdlHandler->init_video() < 0)
		return EXIT_FAILURE;
	if (d->sdlHandler->init_audio() < 0)
		return EXIT_FAILURE;
	d->vBackend = d->sdlHandler->vBackend();

	// Set the window title.
	d->sdlHandler->set_window_title("Gens/GS II [SDL]");

	// Check for startup messages.
	checkForStartupMessages();

	// Set frame timing.
	// TODO: Region code?
	bool isPal = false;
	d->setFrameTiming(isPal ? 50 : 60);

	// TODO: Close the ROM, or let EmuContext do it?

	// Set the color depth.
	MdFb *fb = d->emuContext->m_vdp->MD_Screen->ref();
	fb->setBpp(options->bpp());

	// Set the SDL video source.
	d->sdlHandler->set_video_source(fb);

	// Start audio.
	d->sdlHandler->pause_audio(false);

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
	d->running = true;
	d->paused.data = 0;
	d->last_paused.data = 0;
	while (d->running) {
		// Process the SDL event queue.
		processSdlEventQueue();
		if (!d->running) {
			// Emulation has stopped.
			break;
		}

		// Check if the 'paused' state was changed.
		// If it was, autosave SRAM/EEPROM.
		if (d->last_paused.data != d->paused.data) {
			// 'paused' state was changed.
			// (TODO: Only if paused == true?)
			// TODO: Evaluate both fields as boolean,
			// so switching from manual to manual+auto
			// or vice-versa doesn't trigger an autosave?
			d->emuContext->autoSaveData(-1);
			d->last_paused.data = d->paused.data;
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

		// Autosave SRAM/EEPROM.
		// TODO: EmuContext::execFrame() should probably do this itself...
		d->emuContext->autoSaveData(1);

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
	d->sdlHandler->pause_audio(true);
	usleep(50000);

	// NOTE: Deleting sdlHandler can cause crashes on Windows
	// due to the timer callback trying to post the semaphore
	// after it's been deleted.
	// Shut down the SDL functions manually.
	d->sdlHandler->end_audio();
	d->sdlHandler->end_video();
	d->vBackend = nullptr;

	// Done running the emulation loop.
	return 0;
}

/**
 * Run a normal frame.
 * This function is called by runFrame(),
 * and should be handled by running a full
 * frame with video and audio updates.
 */
void EmuLoop::runFullFrame(void)
{
	EmuLoopPrivate *const d = d_func();
	d->emuContext->execFrame();
}

/**
 * Run a fast frame.
 * This function is called by runFrame() if the
 * system is lagging a bit, and should be handled
 * by running a frame with audio updates only.
 */
void EmuLoop::runFastFrame(void)
{
	EmuLoopPrivate *const d = d_func();
	d->emuContext->execFrameFast();
}

}
