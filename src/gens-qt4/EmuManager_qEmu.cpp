/******************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                     *
 * EmuManager_qEmu.cpp: Emulation manager. (Emulation Queue functions.)       *
 *                                                                            *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                         *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                                *
 * Copyright (c) 2008-2015 by David Korth.                                    *
 *                                                                            *
 * This program is free software; you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software                *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA *
 ******************************************************************************/

#include "EmuManager.hpp"
#include "gqt4_main.hpp"

// C includes. (C++ namespace)
#include <cstring>

// ZOMG savestate handler.
#include "libzomg/Zomg.hpp"

// LibGens includes.
#include "libgens/EmuContext/EmuContext.hpp"
using LibGens::EmuContext;

// LibGens video includes.
#include "libgens/Vdp/Vdp.hpp"
#include "libgens/Util/MdFb.hpp"
#include "libgens/Util/Screenshot.hpp"
using LibGens::Vdp;
using LibGens::MdFb;
using LibGens::Screenshot;

// LibGens CPU includes.
#include "libgens/cpu/M68K.hpp"
#include "libgens/cpu/Z80.hpp"

// Audio backend.
#include "Audio/GensPortAudio.hpp"

// Qt includes.
#include <QtCore/QBuffer>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QVariant>
#include <QtCore/QIODevice>
#include <QtGui/QApplication>
#include <QtGui/QImage>
#include <QtGui/QImageReader>

// LibZomg's image writer class.
#include "libzomg/img_data.h"

namespace GensQt4 {

/** Emulation Request Queue: Submission functions. **/

/**
 * Request a screenshot.
 */
void EmuManager::screenShot(void)
{
	if (!m_rom)
		return;

	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_SCREENSHOT;
	m_qEmuRequest.enqueue(rq);

	if (m_paused.data)
		processQEmuRequest();
}

/**
 * Set the audio sampling rate.
 * @param newRate New audio sampling rate.
 */
void EmuManager::setAudioRate(int newRate)
{
	if (newRate > LibGens::SoundMgr::MAX_SAMPLING_RATE)
		return;

	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_AUDIO_RATE;
	rq.audioRate = newRate;
	m_qEmuRequest.enqueue(rq);

	if (!m_rom || m_paused.data)
		processQEmuRequest();
}

/**
 * Set stereo or mono.
 * @param newStereo True for stereo; false for mono.
 */
void EmuManager::setStereo(bool newStereo)
{
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_AUDIO_STEREO;
	rq.audioStereo = newStereo;
	m_qEmuRequest.enqueue(rq);

	if (!m_rom || m_paused.data)
		processQEmuRequest();
}

/**
 * Reset a CPU.
 * @param cpu_idx CPU index.
 * TODO: Use MDP CPU indexes.
 */
void EmuManager::resetCpu(int cpu_idx)
{
	if (!m_rom)
		return;

	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_RESET_CPU;
	rq.cpu_idx = (ResetCpuIndex)cpu_idx;
	m_qEmuRequest.enqueue(rq);

	if (m_paused.data)
		processQEmuRequest();
}

/**
 * Save the current emulation state.
 */
void EmuManager::saveState(void)
{
	if (!m_rom)
		return;

	// Get the savestate filename.
	QString filename = getSaveStateFilename();

	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_SAVE_STATE;
	rq.saveState.filename = new QString(filename);
	rq.saveState.saveSlot = m_saveSlot;
	m_qEmuRequest.enqueue(rq);

	if (m_paused.data)
		processQEmuRequest();
}

/**
 * Load the emulation state from a file.
 */
void EmuManager::loadState(void)
{
	if (!m_rom)
		return;

	// Get the savestate filename.
	QString filename = getSaveStateFilename();

	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_LOAD_STATE;
	rq.saveState.filename = new QString(filename);
	rq.saveState.saveSlot = m_saveSlot;
	m_qEmuRequest.enqueue(rq);

	if (m_paused.data)
		processQEmuRequest();
}

/**
 * Set the paused state.
 * @param paused_set Paused flags to set.
 * @param paused_clear Paused flags to clear.
 */
void EmuManager::pauseRequest(paused_t paused_set, paused_t paused_clear)
{
	paused_t newPause = m_paused;
	newPause.data |= paused_set.data;
	newPause.data &= ~paused_clear.data;

	// Toggle the paused state.
	pauseRequest(newPause);
}

/**
 * Set the manual paused state.
 * @param newManualPaused New manual paused state.
 */
void EmuManager::pauseRequest(bool newManualPaused)
{
	paused_t newPause = m_paused;
	newPause.paused_manual = !!newManualPaused;
	pauseRequest(newPause);
}

/**
 * Set the paused state.
 * @param newPaused New paused state.
 */
void EmuManager::pauseRequest(paused_t newPaused)
{
	if (!m_rom)
		return;

	if (!!(m_paused.data) == !!(newPaused.data)) {
		// Paused state is effectively the same.
		// Simply update the emulator state.
		m_paused = newPaused;
		emit stateChanged();
		return;
	}

	// Check if we should pause or unpause the emulator.
	if (!newPaused.data) {
		// Unpause the ROM immediately.
		// TODO: Reset the FPS counter?
		m_paused = newPaused;
		m_audio->open();	// TODO: Add a resume() function.
		if (gqt4_emuThread)
			gqt4_emuThread->resume(false);
		emit stateChanged();
	} else {
		// Queue the pause request.
		EmuRequest_t rq;
		rq.rqType = EmuRequest_t::RQT_PAUSE_EMULATION;
		rq.newPaused = newPaused;
		m_qEmuRequest.enqueue(rq);
	}
}

/**
 * Save slot changed.
 * @param saveSlot (int) Save slot number, (0-9)
 */
void EmuManager::saveSlot_changed_slot(const QVariant &saveSlot)
{
	// NOTE: Don't check if the save slot is the same.
	// This allows users to recheck a savestate's preview image.

	// Queue the save slot request.
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_SAVE_SLOT;
	rq.saveState.saveSlot = saveSlot.toInt();
	m_qEmuRequest.enqueue(rq);

	if (!m_rom || m_paused.data)
		processQEmuRequest();
}

/**
 * Enable SRam/EEPRom setting has changed.
 * @param enableSRam (bool) New Enable SRam/EEPRom setting.
 */
void EmuManager::enableSRam_changed_slot(const QVariant &enableSRam)
{
	// Queue the Enable SRam/EEPRom request.
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_ENABLE_SRAM;
	rq.enableSRam = enableSRam.toBool();
	m_qEmuRequest.enqueue(rq);

	if (!m_rom || m_paused.data)
		processQEmuRequest();
}

/**
 * Change the Auto Fix Checksum setting.
 * @param autoFixChecksum (bool) New Auto Fix Checksum setting.
 */
void EmuManager::autoFixChecksum_changed_slot(const QVariant &autoFixChecksum)
{
	// Queue the autofix checksum change request.
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_AUTOFIX_CHANGE;
	rq.autoFixChecksum = autoFixChecksum.toBool();
	m_qEmuRequest.enqueue(rq);

	if (!m_rom || m_paused.data)
		processQEmuRequest();
}

/**
 * Region code has changed.
 * @param regionCode (int) New region code setting.
 */
void EmuManager::regionCode_changed_slot(const QVariant &regionCode)
{
	// NOTE: Region code change is processed even if a ROM isn't loaded,
	// since we're printing a message to the screen.

	// Queue the region code change.
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_REGION_CODE;
	rq.region = (LibGens::SysVersion::RegionCode_t)regionCode.toInt();
	m_qEmuRequest.enqueue(rq);

	if (!m_rom || m_paused.data)
		processQEmuRequest();
}

/**
 * Region code auto-detection order has changed.
 * @param regionCodeOrder New region code auto-detection order setting.
 */
void EmuManager::regionCodeOrder_changed_slot(const QVariant &regionCodeOrder)
{
	if (!m_rom)
		return;

	// regionCodeOrder isn't actually used here...
	Q_UNUSED(regionCodeOrder);

	// If we're not using region code auto-detection, don't do anything else.
	if (gqt4_cfg->getInt(QLatin1String("System/regionCode")) !=
	    (int)LibGens::SysVersion::REGION_AUTO)
	{
		return;
	}

	// Auto-detect order has changed, and we're currently using auto-detect.
	// Handle it as a regular region code change.
	// Queue the region code change.
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_REGION_CODE;
	rq.region = LibGens::SysVersion::REGION_AUTO;
	m_qEmuRequest.enqueue(rq);

	if (!m_rom || m_paused.data)
		processQEmuRequest();
}

/**
 * Reset the emulator.
 * @param hardReset If true, do a hard reset; otherwise, do a soft reset.
 */
void EmuManager::resetEmulator(bool hardReset)
{
	if (!m_rom)
		return;

	// Queue the reset request.
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_RESET;
	rq.hardReset = hardReset;
	m_qEmuRequest.enqueue(rq);

	if (m_paused.data)
		processQEmuRequest();
}

/**
 * Change a palette setting.
 * @param type Type of palette setting.
 * @param val New value.
 */
void EmuManager::changePaletteSetting(EmuRequest_t::PaletteSettingType type, int val)
{
	// Queue the graphics setting request.
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_PALETTE_SETTING;
	rq.PaletteSettings.ps_type = type;
	rq.PaletteSettings.ps_val = val;
	m_qEmuRequest.enqueue(rq);

	if (!m_rom || m_paused.data)
		processQEmuRequest();
}

/** Graphics settings. **/

void EmuManager::interlacedMode_changed_slot(const QVariant &interlacedMode)
	{ changePaletteSetting(EmuRequest_t::RQT_PS_INTERLACEDMODE, interlacedMode.toInt()); }

/** VDP settings. **/

void EmuManager::borderColorEmulation_changed_slot(const QVariant &borderColorEmulation)
	{ changePaletteSetting(EmuRequest_t::RQT_PS_BORDERCOLOREMULATION, (int)borderColorEmulation.toBool()); }
void EmuManager::ntscV30Rolling_changed_slot(const QVariant &ntscV30Rolling)
	{ changePaletteSetting(EmuRequest_t::RQT_PS_NTSCV30ROLLING, (int)ntscV30Rolling.toBool()); }
void EmuManager::spriteLimits_changed_slot(const QVariant &spriteLimits)
	{ changePaletteSetting(EmuRequest_t::RQT_PS_SPRITELIMITS, (int)spriteLimits.toBool()); }
void EmuManager::zeroLengthDMA_changed_slot(const QVariant &zeroLengthDMA)
	{ changePaletteSetting(EmuRequest_t::RQT_PS_ZEROLENGTHDMA, (int)zeroLengthDMA.toBool()); }
void EmuManager::vscrollBug_changed_slot(const QVariant &vscrollBug)
	{ changePaletteSetting(EmuRequest_t::RQT_PS_VSCROLLBUG, (int)vscrollBug.toBool()); }
void EmuManager::updatePaletteInVBlankOnly_changed_slot(const QVariant &updatePaletteInVBlankOnly)
	{ changePaletteSetting(EmuRequest_t::RQT_PS_UPDATEPALETTEINVBLANKONLY, (int)updatePaletteInVBlankOnly.toBool()); }
void EmuManager::enableInterlacedMode_changed_slot(const QVariant &enableInterlacedMode)
	{ changePaletteSetting(EmuRequest_t::RQT_PS_ENABLEINTERLACEDMODE, (int)enableInterlacedMode.toBool()); }

/** Emulation Request Queue: Processing functions. **/

/**
 * Process the Emulation Request queue.
 */
void EmuManager::processQEmuRequest(void)
{
	while (!m_qEmuRequest.isEmpty()) {
		EmuRequest_t rq = m_qEmuRequest.dequeue();

		switch (rq.rqType) {
			case EmuRequest_t::RQT_SCREENSHOT:
				// Screenshot.
				doScreenShot();
				break;

			case EmuRequest_t::RQT_AUDIO_RATE:
				// Audio sampling rate.
				doAudioRate(rq.audioRate);
				break;

			case EmuRequest_t::RQT_AUDIO_STEREO:
				// Stereo / Mono.
				doAudioStereo(rq.audioStereo);
				break;

			case EmuRequest_t::RQT_SAVE_STATE:
				// Save a savestate.
				doSaveState(*rq.saveState.filename, rq.saveState.saveSlot);
				delete rq.saveState.filename;
				break;

			case EmuRequest_t::RQT_LOAD_STATE:
				// Load a savestate.
				doLoadState(*rq.saveState.filename, rq.saveState.saveSlot);
				delete rq.saveState.filename;
				break;

			case EmuRequest_t::RQT_SAVE_SLOT:
				// Set the save slot.
				doSaveSlot(rq.saveState.saveSlot);
				break;

			case EmuRequest_t::RQT_PAUSE_EMULATION:
				// Pause emulation.
				// Unpausing emulation is handled in EmuManager::pauseRequest().
				doPauseRequest(rq.newPaused);
				break;

			case EmuRequest_t::RQT_RESET:
				doResetEmulator(rq.hardReset);
				break;

			case EmuRequest_t::RQT_AUTOFIX_CHANGE:
				// Set the Auto Fix Checksum setting.
				// TODO: Apply changes immediately?
				EmuContext::SetAutoFixChecksum(rq.autoFixChecksum);
				break;

			case EmuRequest_t::RQT_PALETTE_SETTING:
				// Set a palette setting.
				doChangePaletteSetting(rq.PaletteSettings.ps_type, rq.PaletteSettings.ps_val);
				break;

			case EmuRequest_t::RQT_RESET_CPU:
				// Reset a CPU.
				doResetCpu(rq.cpu_idx);
				break;

			case EmuRequest_t::RQT_REGION_CODE:
				// Set the region code.
				doRegionCode(rq.region);
				break;

			case EmuRequest_t::RQT_ENABLE_SRAM:
				// Enable/disable SRam/EEPRom.
				doEnableSRam(rq.enableSRam);
				break;

			case EmuRequest_t::RQT_UNKNOWN:
			default:
				// Unknown emulation request.
				break;
		}
	}
}

/**
 * Do a screenshot.
 * Called from processQEmuRequest().
 */
void EmuManager::doScreenShot(void)
{
	// Get the ROM filename (without extension).
	// TODO: Remove all extensions, not just the base?
	// Otherwise, S1.bin.gz will save as S1.bin_000.png.
	const QString romFilename = QString::fromUtf8(m_rom->filenameBaseNoExt().c_str());

	// Add the current directory, number, and .png extension.
	// TODO: Enumerate QImageWriter for supported image formats.
	const QString scrFilenamePrefix =
		gqt4_cfg->configPath(PathConfig::GCPATH_SCREENSHOTS) + romFilename;
	const QString scrFilenameSuffix = QLatin1String(".png");
	QString scrFilename;
	int scrNumber = -1;
	do {
		// TODO: Figure out how to optimize this!
		scrNumber++;
		scrFilename = scrFilenamePrefix + QChar(L'_') +
				QString::number(scrNumber).rightJustified(3, QChar(L'0')) +
				scrFilenameSuffix;
	} while (QFile::exists(scrFilename));

	// Take the screenshot.
	MdFb *fb = gqt4_emuContext->m_vdp->MD_Screen->ref();
	int ret = Screenshot::toFile(fb, m_rom, scrFilename.toUtf8().constData());
	fb->unref();

	// Done using the framebuffer.
	fb->unref();

	QString osdMsg;
	if (ret == 0) {
		//: OSD message indicating a screenshot has been saved.
		osdMsg = tr("Screenshot %1 saved.");
		osdMsg = osdMsg.arg(scrNumber);
	} else {
		// TODO: Print the actual error.
		//: OSD message indicating an error occurred while saving a screenshot.
		osdMsg = tr("Error saving screenshot: %1").arg(QLatin1String(strerror(-ret)));
	}

	emit osdPrintMsg(1500, osdMsg);
}

/**
 * Set the audio sampling rate.
 * @param newRate New audio sampling rate.
 */
void EmuManager::doAudioRate(int newRate)
{
	if (m_audio->rate() == newRate)
		return;
	m_audio->setRate(newRate);

	//: OSD message indicating sampling rate change.
	QString osdMsg = tr("Audio sampling rate set to %L1 Hz.", "osd");
	osdMsg = osdMsg.arg(newRate);

	// Emit the signal.
	emit osdPrintMsg(1500, osdMsg);
}

/**
 * Set stereo or mono.
 * @param newStereo True for stereo; false for mono.
 */
void EmuManager::doAudioStereo(bool newStereo)
{
	if (m_audio->isStereo() == newStereo)
		return;
	m_audio->setStereo(newStereo);

	//: OSD message indicating audio stereo/mono change.
	QString osdMsg = tr("Audio set to %1.", "osd");
	osdMsg = osdMsg.arg(newStereo
		//: OSD message indicating audio has been set to Stereo.
		? tr("Stereo", "osd")
		//: OSD message indicating audio has been set to Mono.
		: tr("Mono", "osd"));
	emit osdPrintMsg(1500, osdMsg);
}

/**
 * Save the current emulation state to a file.
 * @param filename Filename.
 * @param saveSlot Save slot number. (0-9)
 */
void EmuManager::doSaveState(QString filename, int saveSlot)
{
	// Save the ZOMG file.
	const QString nativeFilename = QDir::toNativeSeparators(filename);
	int ret = gqt4_emuContext->zomgSave(nativeFilename.toUtf8().constData());

	QString osdMsg;
	if (ret == 0) {
		// Savestate saved.
		if (saveSlot >= 0) {
			//: OSD message indicating a savestate has been saved.
			osdMsg = tr("State %1 saved.", "osd").arg(saveSlot);
		} else {
			//: OSD message indicating a savestate has been saved using a specified filename.
			osdMsg = tr("State saved in %1", "osd").arg(nativeFilename);
		}
	} else {
		// Error saving savestate.
		//: OSD message indicating an error occurred while saving the savestate.
		osdMsg = tr("Error saving state: %1", "osd").arg(ret);
	}

	// Print the message to the OSD.
	emit osdPrintMsg(1500, osdMsg);
}

/**
 * Load the emulation state from a file.
 * @param filename Filename.
 * @param saveSlot Save slot number. (0-9)
 */
void EmuManager::doLoadState(QString filename, int saveSlot)
{
	// TODO: Redraw the screen if emulation is paused.

	// Load the ZOMG file.
	const QString nativeFilename = QDir::toNativeSeparators(filename);
	int ret = gqt4_emuContext->zomgLoad(nativeFilename.toUtf8().constData());

	QString osdMsg;
	if (ret == 0) {
		// Savestate loaded.
		if (saveSlot >= 0) {
			//: OSD message indicating a savestate has been loaded.
			osdMsg = tr("State %1 loaded.", "osd").arg(saveSlot);
		} else {
			//: OSD message indicating a savestate has been loaded using a specified filename.
			osdMsg = tr("State loaded from %1", "osd").arg(nativeFilename);
		}
	} else {
		// Error loading savestate.
		//: OSD message indicating an error occurred while loading the savestate.
		osdMsg = tr("Error loading state: %1", "osd").arg(ret);
	}

	// Print the message to the OSD.
	emit osdPrintMsg(1500, osdMsg);
}

/**
 * Set the current savestate slot.
 * @param newSaveSlot Save slot number. (0-9)
 */
void EmuManager::doSaveSlot(int newSaveSlot)
{
	if (newSaveSlot < 0 || newSaveSlot > 9)
		return;
	m_saveSlot = newSaveSlot;

	// Standard OSD duration for save slot selection.
	static const int OsdDuration = 1500;

	// Preview image.
	QImage imgPreview;

	if (!m_rom) {
		// No ROM is loaded.
		//: OSD message indicating a save slot is selected while no ROM is loaded.
		QString osdMsg = tr("Save Slot %1 selected.", "osd").arg(m_saveSlot);
		emit osdPrintMsg(OsdDuration, osdMsg);
		emit osdShowPreview(0, imgPreview);
		return;
	}

	// ROM is loaded.
	//: OSD message indicating a save slot is selected while a ROM is loaded.
	QString osdMsg = tr("Save Slot %1 [%2]", "osd").arg(m_saveSlot);

	// Check if the file exists.
	QString filename = getSaveStateFilename();
	if (QFile::exists(filename)) {
		// Savestate exists.
		//: OSD message indicating a savestate exists in the selected slot.
		osdMsg = osdMsg.arg(tr("OCCUPIED", "osd"));

		// Attempt to load a preview image from the savestate.
		QString nativeFilename = QDir::toNativeSeparators(filename);
		LibZomg::Zomg zomg(nativeFilename.toUtf8().constData(), LibZomg::Zomg::ZOMG_LOAD);

		Zomg_Img_Data_t img_data;
		int ret = zomg.loadPreview(&img_data);
		if (ret == 0) {
			// Preview image loaded from the ZOMG file.
			// TODO: If we construct a QImage using the raw data directly,
			// a copy won't be needed, but it won't be deleted.
			// Figure out a more efficient way to handle this.
			// (Store imgPreview locally?)
			// TODO: Add LOAD parameters to img_data for e.g. not swapping BGR.
			imgPreview = QImage(img_data.w, img_data.h, QImage::Format_RGB32);
			memcpy(imgPreview.bits(), img_data.data, img_data.pitch * img_data.h);
			free(img_data.data);
		}

		// Close the savestate.
		zomg.close();
	} else {
		// Savestate doesn't exist.
		//: OSD message indicating there is no savestate in the selected slot.
		osdMsg = osdMsg.arg(tr("EMPTY", "osd"));
	}

	// Print the OSD.
	emit osdPrintMsg(OsdDuration, osdMsg);
	emit osdShowPreview((imgPreview.isNull() ? 0 : OsdDuration), imgPreview);
}

/**
 * Pause emulation.
 * Unpausing emulation is handled in EmuManager::pauseRequest().
 * @param newPaused New paused state.
 */
void EmuManager::doPauseRequest(paused_t newPaused)
{
	if (m_paused.data) {
		// Emulator is already paused.
		// Simply update the paused state.
		m_paused = newPaused;
		emit stateChanged();
		return;
	}

	// Pause the emulator.
	m_paused = newPaused;

	// Turn off audio and autosave SRam/EEPRom.
	m_audio->close();	// TODO: Add a pause() function.
	gqt4_emuContext->autoSaveData(-1);

	// Emulation state has changed.
	emit stateChanged();
}

/**
 * Reset the emulator.
 * @param hardReset If true, do a hard reset; otherwise, do a soft reset.
 */
void EmuManager::doResetEmulator(bool hardReset)
{
	QString msg;
	if (hardReset) {
		// Do a hard reset.
		gqt4_emuContext->hardReset();

		//: OSD message indicating a Hard Reset was performed.
		msg = tr("Hard Reset.", "osd");
	} else {
		// Do a soft reset.
		gqt4_emuContext->softReset();

		//: OSD message indicating a Soft Reset was performed.
		msg = tr("Soft Reset.", "osd");
	}

	// Print the message to the OSD.
	emit osdPrintMsg(2500, msg);
}


/**
 * Change a palette setting.
 * @param type Type of palette setting.
 * @param val New value.
 */
void EmuManager::doChangePaletteSetting(EmuRequest_t::PaletteSettingType type, int val)
{
	// TODO: Initialize palette settings on ROM startup.
	// TODO: These should be context-specific.
	if (!gqt4_emuContext)
		return;
	LibGens::VdpTypes::VdpEmuOptions_t *options = &gqt4_emuContext->m_vdp->options;

	switch (type) {
		case EmuRequest_t::RQT_PS_INTERLACEDMODE: {
			// Interlaced Mode isn't exactly a "palette" setting.
			// TODO: Rename to "VDP setting"?
			// TODO: Consolidate the two interlaced rendering mode enums.
			options->intRendMode = ((LibGens::VdpTypes::IntRend_Mode_t)val);

			// Gens/GS r7+ prints a message to the OSD, so we'll do that too.
			//: OSD message indicating the interlaced rendering mode was changed.
			QString msg = tr("Interlaced: %1", "osd");
			switch (options->intRendMode) {
				case LibGens::VdpTypes::INTREND_EVEN:
					//: OSD message indicating the interlaced rendering mode was set to even lines only.
					msg = msg.arg(tr("Even lines only", "osd"));
					break;
				case LibGens::VdpTypes::INTREND_ODD:
					//: OSD message indicating the interlaced rendering mode was set to odd lines only.
					msg = msg.arg(tr("Odd lines only", "osd"));
					break;
				case LibGens::VdpTypes::INTREND_FLICKER:
					//: OSD message indicating the interlaced rendering mode was set to alternating lines.
					msg = msg.arg(tr("Alternating lines", "osd"));
					break;
				case LibGens::VdpTypes::INTREND_2X:
					//: OSD message indicating the interlaced rendering mode was set to 2x resolution.
					msg = msg.arg(tr("2x resolution", "osd"));
					break;
				default:
					msg.clear();
					break;
			}

			// Print the message to the OSD.
			emit osdPrintMsg(1500, msg);
			break;
		}

		case EmuRequest_t::RQT_PS_BORDERCOLOREMULATION:
			options->borderColorEmulation = !!val;
			break;
		case EmuRequest_t::RQT_PS_NTSCV30ROLLING:
			options->ntscV30Rolling = !!val;
			break;
		case EmuRequest_t::RQT_PS_SPRITELIMITS:
			options->spriteLimits = !!val;
			break;
		case EmuRequest_t::RQT_PS_ZEROLENGTHDMA:
			options->zeroLengthDMA = !!val;
			break;
		case EmuRequest_t::RQT_PS_VSCROLLBUG:
			options->vscrollBug = !!val;
			break;
		case EmuRequest_t::RQT_PS_UPDATEPALETTEINVBLANKONLY:
			options->updatePaletteInVBlankOnly = !!val;
			break;
		case EmuRequest_t::RQT_PS_ENABLEINTERLACEDMODE:
			options->enableInterlacedMode = !!val;
			break;
		default:
			break;
	}
}

/**
 * Reset a CPU.
 * @param cpu_idx CPU index.
 * TODO: Use MDP CPU indexes.
 */
void EmuManager::doResetCpu(EmuManager::ResetCpuIndex cpu_idx)
{
	// TODO: Reset CPUs through the emulation context using MDP CPU indexes.
	QString msg;
	switch (cpu_idx) {
		case RQT_CPU_M68K:
			LibGens::M68K::Reset();
			//: OSD message indicating the 68000 CPU was reset.
			msg = tr("68000 reset.", "osd");
			break;

		case RQT_CPU_Z80:
			LibGens::Z80::SoftReset();
			// TODO: Add Z80 hard reset option?
			//: OSD message indicating the Z80 CPU was reset.
			msg = tr("Z80 reset.", "osd");
			break;

		default:
			msg.clear();
			break;
	}

	// Print the message to the OSD.
	emit osdPrintMsg(1500, msg);
}

/**
 * Set the region code.
 * @param region New region code setting.
 */
void EmuManager::doRegionCode(LibGens::SysVersion::RegionCode_t region)
{
	// TODO: Verify if this is an actual change.
	// If it isn't, don't do anything.

	// Print a message to the OSD.
	const QString region_str = LgRegionCodeStr(region);
	//: OSD message indicating the system region code was changed.
	const QString str = tr("System region set to %1.", "osd");
	emit osdPrintMsg(1500, str.arg(region_str));

	// NOTE: Region code order validation isn't needed here,
	// since it's done elsewhere.
	if (m_rom) {
		// Emulation is running. Change the region.
		// GetLgRegionCode() is needed for region auto-detection.
		LibGens::SysVersion::RegionCode_t lg_region = GetLgRegionCode(
					region, m_rom->regionCode(),
					(uint16_t)gqt4_cfg->getUInt(QLatin1String("System/regionCodeOrder")));
		gqt4_emuContext->setRegion(lg_region);

		if (region == LibGens::SysVersion::REGION_AUTO) {
			// Print the auto-detected region.
			const QString detect_str = LgRegionCodeStr(lg_region);
			if (!detect_str.isEmpty()) {
				//: OSD message indicating the auto-detected ROM region.
				const QString auto_str = tr("ROM region detected as %1.", "osd");
				emit osdPrintMsg(1500, auto_str.arg(detect_str));
			}
		}
	}

	// Update the system name in the GensWindow title bar.
	emit stateChanged();
}

/**
 * Enable/disable SRam/EEPRom.
 * @param enableSRam New enable/disable SRam/EEPRom value.
 */
void EmuManager::doEnableSRam(bool enableSRam)
{
	if (gqt4_emuContext) {
		if (gqt4_emuContext->saveDataEnable() == enableSRam) {
			// SRAM enabled value is the same.
			// Don't show a message.
			return;
		}

		// Set the SRAM enabled value.
		gqt4_emuContext->setSaveDataEnable(enableSRam);
	}

	// Print a message to the screen.
	QString msg;
	if (enableSRam) {
		//: OSD message indicating SRAM/EEPROM has been enabled.
		msg = tr("SRAM/EEPROM enabled.", "osd");
	} else {
		//: OSD message indicating SRAM/EEPROM has been disabled.
		msg = tr("SRAM/EEPROM disabled.", "osd");
	}

	// Print the message to the OSD.
	emit osdPrintMsg(1500, msg);
}

}
