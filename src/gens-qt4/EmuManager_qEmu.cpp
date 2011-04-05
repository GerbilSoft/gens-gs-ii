/******************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                     *
 * EmuManager_qEmu.cpp: Emulation manager. (Emulation Queue functions.)       *
 *                                                                            *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                         *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                                *
 * Copyright (c) 2008-2011 by David Korth.                                    *
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

// ZOMG savestate handler.
#include "libgens/Save/GensZomg.hpp"

// LibGens includes.
#include "libgens/Util/Timing.hpp"
#include "libgens/MD/EmuMD.hpp"

// LibGens video includes.
#include "libgens/MD/VdpRend.hpp"
#include "libgens/MD/VdpIo.hpp"
#include "libgens/MD/TAB336.h"
#include "libgens/MD/VdpRend_m5.hpp"

// Controller devices.
#include "libgens/IO/IoBase.hpp"
#include "libgens/IO/Io3Button.hpp"
#include "libgens/IO/Io6Button.hpp"
#include "libgens/IO/Io2Button.hpp"
#include "libgens/IO/IoMegaMouse.hpp"
#include "libgens/IO/IoTeamplayer.hpp"
#include "libgens/IO/Io4WPMaster.hpp"
#include "libgens/IO/Io4WPSlave.hpp"

// LibGens CPU includes.
#include "libgens/cpu/M68K.hpp"
#include "libgens/cpu/Z80.hpp"

// Audio backend.
#include "Audio/GensPortAudio.hpp"

// Qt includes.
#include <QtCore/QBuffer>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtGui/QApplication>
#include <QtGui/QImage>
#include <QtGui/QImageReader>
#include <QtGui/QImageWriter>
#include <QtGui/QMessageBox>


namespace GensQt4
{

/** Emulation Request Queue: Submission functions. **/

/**
 * screenShot(): Request a screenshot.
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
 * setAudioRate(): Set the audio sampling rate.
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
 * setStereo(): Set stereo or mono.
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
 * resetCpu(): Reset a CPU.
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
 * saveState(): Save the current emulation state.
 */
void EmuManager::saveState(void)
{
	if (!m_rom)
		return;
	
	// Get the savestate filename.
	QString filename = getSaveStateFilename();
	
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_SAVE_STATE;
	rq.saveState.filename = strdup(filename.toUtf8().constData());
	rq.saveState.saveSlot = m_saveSlot;
	m_qEmuRequest.enqueue(rq);
	
	if (m_paused.data)
		processQEmuRequest();
}

/**
 * loadState(): Load the emulation state from a file.
 */
void EmuManager::loadState(void)
{
	if (!m_rom)
		return;
	
	// Get the savestate filename.
	QString filename = getSaveStateFilename();
	
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_LOAD_STATE;
	rq.saveState.filename = strdup(filename.toUtf8().constData());
	rq.saveState.saveSlot = m_saveSlot;
	m_qEmuRequest.enqueue(rq);
	
	if (m_paused.data)
		processQEmuRequest();
}


/**
 * pauseRequest(): Set the paused state.
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
 * pauseRequest(): Set the manual paused state.
 * @param newManualPaused New manual paused state.
 */
void EmuManager::pauseRequest(bool newManualPaused)
{
	paused_t newPause = m_paused;
	newPause.paused_manual = !!newManualPaused;
	pauseRequest(newPause);
}

/**
 * pauseRequest(): Set the paused state.
 * @param newPaused New paused state.
 */
void EmuManager::pauseRequest(paused_t newPaused)
{
	if (!m_rom)
		return;
	
	if (!!(m_paused.data) == !!(newPaused.data))
	{
		// Paused state is effectively the same.
		// Simply update the emulator state.
		m_paused = newPaused;
		emit stateChanged();
		return;
	}
	
	// Check if we should pause or unpause the emulator.
	if (!newPaused.data)
	{
		// Unpause the ROM immediately.
		// TODO: Reset the FPS counter?
		m_paused = newPaused;
		m_audio->open();	// TODO: Add a resume() function.
		if (gqt4_emuThread)
			gqt4_emuThread->resume(false);
		emit stateChanged();
	}
	else
	{
		// Queue the pause request.
		EmuRequest_t rq;
		rq.rqType = EmuRequest_t::RQT_PAUSE_EMULATION;
		rq.newPaused = newPaused;
		m_qEmuRequest.enqueue(rq);
	}
}


/**
 * saveSlot_changed_slot(): Save slot changed.
 * @param newSaveSlot Save slot number, (0-9)
 */
void EmuManager::saveSlot_changed_slot(int newSaveSlot)
{
	// NOTE: Don't check if the save slot is the same.
	// This allows users to recheck a savestate's preview image.
	if (newSaveSlot < 0 || newSaveSlot > 9)
		return;
	
	// Queue the save slot request.
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_SAVE_SLOT;
	rq.saveState.saveSlot = newSaveSlot;
	m_qEmuRequest.enqueue(rq);
	
	if (!m_rom || m_paused.data)
		processQEmuRequest();
}


/**
 * autoFixChecksum_changed_slot(): Change the Auto Fix Checksum setting.
 * @param newAutoFixChecksum New Auto Fix Checksum setting.
 */
void EmuManager::autoFixChecksum_changed_slot(bool newAutoFixChecksum)
{
	// Queue the autofix checksum change request.
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_AUTOFIX_CHANGE;
	rq.autoFixChecksum = newAutoFixChecksum;
	m_qEmuRequest.enqueue(rq);
	
	if (!m_rom || m_paused.data)
		processQEmuRequest();
}


/**
 * regionCode_changed_slot(): Region code has changed.
 * @param newRegionCode New region code setting.
 */
void EmuManager::regionCode_changed_slot(GensConfig::ConfRegionCode_t newRegionCode)
{
	// NOTE: Region code change is processed even if a ROM isn't loaded,
	// since we're printing a message to the screen.
	
	// Queue the region code change.
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_REGION_CODE;
	rq.region = newRegionCode;
	m_qEmuRequest.enqueue(rq);
	
	if (!m_rom || m_paused.data)
		processQEmuRequest();
}


/**
 * regionCodeOrder_changed_slot(): Region code auto-detection order has changed.
 * @param newRegionCodeOrder New region code auto-detection order setting.
 */
void EmuManager::regionCodeOrder_changed_slot(uint16_t newRegionCodeOrder)
{
	if (!m_rom)
		return;
	if (gqt4_config->regionCode() != GensConfig::CONFREGION_AUTODETECT)
		return;
	
	// Auto-detect order has changed, and we're currently using auto-detect.
	// Handle it as a regular region code change.
	// Queue the region code change.
	EmuRequest_t rq;
	rq.rqType = EmuRequest_t::RQT_REGION_CODE;
	rq.region = GensConfig::CONFREGION_AUTODETECT;
	m_qEmuRequest.enqueue(rq);
	
	if (!m_rom || m_paused.data)
		processQEmuRequest();
}


/**
 * resetEmulator(): Reset the emulator.
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
 * changePaletteSetting(): Change a palette setting.
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


/** Emulation Request Queue: Processing functions. **/


/**
 * processQEmuRequest(): Process the Emulation Request queue.
 */
void EmuManager::processQEmuRequest(void)
{
	while (!m_qEmuRequest.isEmpty())
	{
		EmuRequest_t rq = m_qEmuRequest.dequeue();
		
		switch (rq.rqType)
		{
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
				doSaveState(rq.saveState.filename, rq.saveState.saveSlot);
				free(rq.saveState.filename);
				break;
			
			case EmuRequest_t::RQT_LOAD_STATE:
				// Load a savestate.
				doLoadState(rq.saveState.filename, rq.saveState.saveSlot);
				free(rq.saveState.filename);
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
				LibGens::EmuContext::SetAutoFixChecksum(rq.autoFixChecksum);
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
			
			case EmuRequest_t::RQT_LOAD_ROM:
			{
				// Load a ROM.
				int ret = doLoadRom(rq.loadRom.rom);
				if (ret == 0)
				{
					// ROM loaded successfully.
					// Add it to the Recent ROMs list.
					gqt4_config->m_recentRoms->update(
						*rq.loadRom.filename,
						*rq.loadRom.z_filename,
						rq.loadRom.rom->sysId());
				}
				
				// Delete the QStrings.
				delete rq.loadRom.filename;
				delete rq.loadRom.z_filename;
				break;
			}
			
			case EmuRequest_t::RQT_UNKNOWN:
			default:
				// Unknown emulation request.
				break;
		}
	}
}


/**
 * getMDScreen(): Get the MD screen as a QImage.
 * @return QImage of the MD screen.
 */
QImage EmuManager::getMDScreen(void) const
{
	// Create the QImage.
	const uint8_t *start;
	const int startY = ((240 - LibGens::VdpIo::GetVPix()) / 2);
	const int startX = (8 + LibGens::VdpIo::GetHPixBegin());
	const int startPx = (TAB336[startY] + startX);
	int bytesPerLine;
	QImage::Format imgFormat;
	
	if (LibGens::VdpRend::m_palette.bpp() == LibGens::VdpPalette::BPP_32)
	{
		start = (const uint8_t*)(&LibGens::VdpRend::MD_Screen.u32[startPx]);
		bytesPerLine = (336 * sizeof(uint32_t));
		imgFormat = QImage::Format_RGB32;
	}
	else
	{
		start = (const uint8_t*)(&LibGens::VdpRend::MD_Screen.u16[startPx]);
		bytesPerLine = (336 * sizeof(uint16_t));
		if (LibGens::VdpRend::m_palette.bpp() == LibGens::VdpPalette::BPP_16)
			imgFormat = QImage::Format_RGB16;
		else
			imgFormat = QImage::Format_RGB555;
	}
	
	// TODO: Check for errors.
	// TODO: Store timestamp, ROM filename, etc.
	return QImage(start, LibGens::VdpIo::GetHPix(),
			LibGens::VdpIo::GetVPix(),
			bytesPerLine, imgFormat);	
}


/**
 * doScreenShot(): Do a screenshot.
 * Called from processQEmuRequest().
 */
void EmuManager::doScreenShot(void)
{
	// Get the ROM filename (without extension).
	// TODO: Remove all extensions, not just the base?
	// Otherwise, S1.bin.gz will save as S1.bin_000.png.
	const QString romFilename = QString::fromUtf8(m_rom->filenameBaseNoExt());
	
	// Add the current directory, number, and .png extension.
	// TODO: Enumerate QImageWriter for supported image formats.
	const QString scrFilenamePrefix =
		gqt4_config->userPath(GensConfig::GCPATH_SCREENSHOTS) + QChar(L'/') + romFilename;
	const QString scrFilenameSuffix = QLatin1String(".png");
	QString scrFilename;
	int scrNumber = -1;
	do
	{
		// TODO: Figure out how to optimize this!
		scrNumber++;
		scrFilename = scrFilenamePrefix + QChar(L'_') +
				QString::number(scrNumber).rightJustified(3, QChar(L'0')) +
				scrFilenameSuffix;
	} while (QFile::exists(scrFilename));
	
	// Create the QImage.
	QImage img = getMDScreen();
	QImageWriter imgWriter(scrFilename, "png");
	imgWriter.write(img);
	
	//: OSD message indicating screenshot saved.
	QString osdMsg = tr("Screenshot %1 saved.");
	osdMsg = osdMsg.arg(scrNumber);
	emit osdPrintMsg(1500, osdMsg);
}


/**
 * doAudioRate(): Set the audio sampling rate.
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
 * doAudioStereo(): Set stereo or mono.
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
 * doSaveState(): Save the current emulation state to a file.
 * @param filename Filename.
 * @param saveSlot Save slot number. (0-9)
 */
void EmuManager::doSaveState(const char *filename, int saveSlot)
{
	// Create the preview image.
	QImage img = getMDScreen();
	QBuffer imgBuf;
	QImageWriter imgWriter(&imgBuf, "png");
	imgWriter.write(img);
	
	// Save the ZOMG file.
	int ret = LibGens::ZomgSave(filename,			// ZOMG filename.
				gqt4_emuContext,		// Emulation context.
				imgBuf.buffer().constData(),	// Preview image.
				imgBuf.buffer().size()		// Size of preview image.
				);
	
	QString sFilename = QString::fromUtf8(filename);
	QString osdMsg;
	
	if (ret == 0)
	{
		// Savestate saved.
		if (saveSlot >= 0)
		{
			//: OSD message indicating a savestate has been saved.
			osdMsg = tr("State %1 saved.", "osd").arg(saveSlot);
		}
		else
		{
			//: OSD message indicating a savestate has been saved using a specified filename
			osdMsg = tr("State saved in %1", "osd").arg(sFilename);
		}
	}
	else
	{
		// Error saving savestate.
		//: OSD message indicating an error occurred while saving the savestate.
		osdMsg = tr("Error saving state: %1", "osd").arg(ret);
	}
	
	// Print a message on the OSD.
	emit osdPrintMsg(1500, osdMsg);
}


/**
 * doLoadState(): Load the emulation state from a file.
 * @param filename Filename.
 * @param saveSlot Save slot number. (0-9)
 */
void EmuManager::doLoadState(const char *filename, int saveSlot)
{
	// TODO: Redraw the screen if emulation is paused.
	int ret = LibGens::ZomgLoad(filename, gqt4_emuContext);
	
	QString sFilename = QString::fromUtf8(filename);
	QString osdMsg;
	
	if (ret == 0)
	{
		// Savestate loaded.
		if (saveSlot >= 0)
		{
			//: OSD message indicating a savestate has been loaded.
			osdMsg = tr("State %1 loaded.", "osd").arg(saveSlot);
		}
		else
		{
			//: OSD message indicating a savestate has been loaded using a specified filename
			osdMsg = tr("State loaded from %1", "osd").arg(sFilename);
		}
	}
	else
	{
		// Error loading savestate.
		//: OSD message indicating an error occurred while loading the savestate.
		osdMsg = tr("Error loading state: %1", "osd").arg(ret);
	}
	
	// Print a message on the OSD.
	emit osdPrintMsg(1500, osdMsg);
}


/**
 * saveSlot(): Set the current savestate slot.
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
	
	if (!m_rom)
	{
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
	if (QFile::exists(filename))
	{
		// Savestate exists.
		//: OSD message indicating a savestate exists in the selected slot.
		osdMsg = osdMsg.arg(tr("OCCUPIED", "osd"));
		
		// Check if the savestate has a preview image.
		LibZomg::Zomg zomg(filename.toUtf8().constData(), LibZomg::Zomg::ZOMG_LOAD);
		if (zomg.getPreviewSize() > 0)
		{
			// Preview image found.
			QByteArray img_ByteArray;
			img_ByteArray.resize(zomg.getPreviewSize());
			int ret = zomg.loadPreview(img_ByteArray.data(), img_ByteArray.size());
			
			if (ret == 0)
			{
				// Preview image loaded from the ZOMG file.
				
				// Convert the preview image to a QImage.
				QBuffer imgBuf(&img_ByteArray);
				imgBuf.open(QIODevice::ReadOnly);
				QImageReader imgReader(&imgBuf, "png");
				imgPreview = imgReader.read();
			}
		}
		
		// Close the savestate.
		zomg.close();
	}
	else
	{
		// Savestate doesn't exist.
		//: OSD message indicating there is no savestate in the selected slot.
		osdMsg = osdMsg.arg(tr("EMPTY", "osd"));
	}
	
	// Print the OSD.
	emit osdPrintMsg(OsdDuration, osdMsg);
	emit osdShowPreview((imgPreview.isNull() ? 0 : OsdDuration), imgPreview);
}


/**
 * doPauseRequest(): Pause emulation.
 * Unpausing emulation is handled in EmuManager::pauseRequest().
 * @param newPaused New paused state.
 */
void EmuManager::doPauseRequest(paused_t newPaused)
{
	if (m_paused.data)
	{
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
 * doResetEmulator(): Reset the emulator.
 * @param hardReset If true, do a hard reset; otherwise, do a soft reset.
 */
void EmuManager::doResetEmulator(bool hardReset)
{
	// If autofix checksum is enabled, re-fix the checksum.
	// Otherwise, restore the checksum.
	// TODO: Move this call to EmuMD::hardReset() / EmuMD::softReset()?
	// (That'll require setting a static option in EmuContext.)
	// TODO: Automatically fix/unfix checksum when the option is changed?
	if (gqt4_config->autoFixChecksum())
		gqt4_emuContext->fixChecksum();
	else
		gqt4_emuContext->restoreChecksum();
	
	if (hardReset)
	{
		// Do a hard reset.
		gqt4_emuContext->hardReset();
		
		//: OSD message indicating a Hard Reset was performed.
		emit osdPrintMsg(2500, tr("Hard Reset.", "osd"));
	}
	else
	{
		// Do a soft reset.
		gqt4_emuContext->softReset();
		
		//: OSD message indicating a Soft Reset was performed.
		emit osdPrintMsg(2500, tr("Soft Reset.", "osd"));
	}
}


/**
 * doChangePaletteSetting(): Change a palette setting.
 * @param type Type of palette setting.
 * @param val New value.
 */
void EmuManager::doChangePaletteSetting(EmuRequest_t::PaletteSettingType type, int val)
{
	// NOTE: gens-qt4 uses values [-100,100] for contrast and brightness.
	// libgens uses [0,200] for contrast and brightness.
	
	switch (type)
	{
		case EmuRequest_t::RQT_PS_CONTRAST:
			LibGens::VdpRend::m_palette.setContrast(val + 100);
			break;
		
		case EmuRequest_t::RQT_PS_BRIGHTNESS:
			LibGens::VdpRend::m_palette.setBrightness(val + 100);
			break;
		
		case EmuRequest_t::RQT_PS_GRAYSCALE:
			LibGens::VdpRend::m_palette.setGrayscale((bool)(!!val));
			break;
		
		case EmuRequest_t::RQT_PS_INVERTED:
			LibGens::VdpRend::m_palette.setInverted((bool)(!!val));
			break;
		
		case EmuRequest_t::RQT_PS_COLORSCALEMETHOD:
			LibGens::VdpRend::m_palette.setColorScaleMethod(
						(LibGens::VdpPalette::ColorScaleMethod_t)val);
			break;
		
		case EmuRequest_t::RQT_PS_INTERLACEDMODE:
		{
			// Interlaced Mode isn't exactly a "palette" setting.
			// TODO: Rename to "VDP setting"?
			// TODO: Consolidate the two interlaced rendering mode enums.
			LibGens::VdpRend_m5::IntRend_Mode =
					((LibGens::VdpRend_m5::IntRend_Mode_t)val);
			
			// Gens/GS r7+ prints a message to the OSD, so we'll do that too.
			//: OSD message indicating the interlaced rendering mode was changed.
			QString msg = tr("Interlaced: %1", "osd");
			switch (LibGens::VdpRend_m5::IntRend_Mode)
			{
				case LibGens::VdpRend_m5::INTREND_EVEN:
					//: OSD message indicating the interlaced rendering mode was set to even lines only.
					msg = msg.arg(tr("Even lines only", "osd"));
					break;
				
				case LibGens::VdpRend_m5::INTREND_ODD:
					//: OSD message indicating the interlaced rendering mode was set to odd lines only.
					msg = msg.arg(tr("Odd lines only", "osd"));
					break;
				
				case LibGens::VdpRend_m5::INTREND_FLICKER:
					//: OSD message indicating the interlaced rendering mode was set to alternating lines.
					msg = msg.arg(tr("Alternating lines", "osd"));
					break;
				
				case LibGens::VdpRend_m5::INTREND_2X:
					//: OSD message indicating the interlaced rendering mode was set to 2x resolution.
					msg = msg.arg(tr("2x resolution", "osd"));
					break;
				
				default:
					msg.clear();
					break;
			}
			
			if (!msg.isEmpty())
				emit osdPrintMsg(1500, msg);
			break;
		}
		
		default:
			break;
	}
}


/**
 * doResetCpu(): Reset a CPU.
 * @param cpu_idx CPU index.
 * TODO: Use MDP CPU indexes.
 */
void EmuManager::doResetCpu(EmuManager::ResetCpuIndex cpu_idx)
{
	// TODO: Reset CPUs through the emulation context using MDP CPU indexes.
	QString msg;
	switch (cpu_idx)
	{
		case RQT_CPU_M68K:
			LibGens::M68K::Reset();
			//: OSD message indicating the 68000 CPU was reset.
			msg = tr("68000 reset.", "osd");
			break;
		
		case RQT_CPU_Z80:
			LibGens::Z80::Reset();
			//: OSD message indicating the Z80 CPU was reset.
			msg = tr("Z80 reset.", "osd");
			break;
		
		default:
			msg.clear();
			break;
	}
	
	if (!msg.isEmpty())
		emit osdPrintMsg(1500, msg);
}


/**
 * doRegionCode(): Set the region code.
 * @param region New region code setting.
 */
void EmuManager::doRegionCode(GensConfig::ConfRegionCode_t region)
{
	// TODO: Verify if this is an actual change.
	// If it isn't, don't do anything.
	
	// Print a message to the OSD.
	const QString region_str = GcRegionCodeStr(region);
	//: OSD message indicating the system region code was changed.
	const QString str = tr("System region set to %1.", "osd");
	emit osdPrintMsg(1500, str.arg(region_str));
	
	if (m_rom)
	{
		// Emulation is running. Change the region.
		LibGens::SysVersion::RegionCode_t lg_region = GetLgRegionCode(
					region, m_rom->regionCode(),
					gqt4_config->regionCodeOrder());
		gqt4_emuContext->setRegion(lg_region);
		
		if (region == GensConfig::CONFREGION_AUTODETECT)
		{
			// Print the auto-detected region.
			const QString detect_str = LgRegionCodeStr(lg_region);
			if (!detect_str.isEmpty())
			{
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
 * doLoadRom(): Load a ROM file.
 * @param rom Previously-opened ROM class.
 * @return 0 on success; non-zero on error.
 */
int EmuManager::doLoadRom(LibGens::Rom *rom)
{
	// Check if this is a multi-file ROM archive.
	if (rom->isMultiFile() && !rom->isRomSelected())
	{
		// Multi-file ROM archive, but a ROM hasn't been selected.
		return -2;
	}
	
	// Make sure the ROM is supported.
	const QChar chrBullet(0x2022);  // U+2022: BULLET
	const QChar chrNewline(L'\n');
	const QChar chrSpace(L' ');
	
	// Check the system ID.
	if (rom->sysId() != LibGens::Rom::MDP_SYSTEM_MD)
	{
		// Only MD ROM images are supported.
		const LibGens::Rom::MDP_SYSTEM_ID errSysId = rom->sysId();
		delete rom;
		
		// TODO: Specify GensWindow as parent window.
		// TODO: Move this out of EmuManager and simply use return codes?
		// (how would we indicate what system the ROM is for...)
		QMessageBox::critical(NULL,
				//: A ROM image was selected for a system that Gens/GS II does not currently support. (error title)
				tr("Unsupported System"),
				//: A ROM image was selected for a system that Gens/GS II does not currently support. (error description)
				tr("The selected ROM image is designed for a system that"
				   " is not currently supported by Gens/GS II.") +
				chrNewline + chrNewline +
				//: Indicate what system the ROM image is for.
				tr("Selected ROM's system: %1").arg(SysName_l(errSysId)) +
				chrNewline + chrNewline +
				//: List of systems that Gens/GS II currently supports.
				tr("Supported systems:") + chrNewline +
				chrBullet + chrSpace + SysName_l(LibGens::Rom::MDP_SYSTEM_MD)
				);
		
		return 3;
	}
	
	// Check the ROM format.
	if (rom->romFormat() != LibGens::Rom::RFMT_BINARY)
	{
		// Only binary ROM images are supported.r
		LibGens::Rom::RomFormat errRomFormat = rom->romFormat();
		delete rom;
		
		// Get the ROM format.
		QString sRomFormat = RomFormat(errRomFormat);
		if (sRomFormat.isEmpty())
		{
			//: Unknown ROM format. (EmuManager::RomFormat() returned an empty string.)
			sRomFormat = tr("(unknown)", "rom-format");
		}
		
		// TODO: Specify GensWindow as parent window.
		// TODO: Move this out of EmuManager and simply use return codes?
		// (how would we indicate what format the ROM was in...)
		QMessageBox::critical(NULL,
				//: A ROM image was selected in a format that Gens/GS II does not currently support. (error title)
				tr("Unsupported ROM Format"),
				//: A ROM image was selected in a format that Gens/GS II does not currently support. (error description)
				tr("The selected ROM image is in a format that is not currently supported by Gens/GS II.") +
				chrNewline + chrNewline +
				//: Indicate what format the ROM image is in.
				tr("Selected ROM image format: %1").arg(sRomFormat) +
				chrNewline + chrNewline +
				//: List of ROM formats that Gens/GS II currently supports.
				tr("Supported ROM formats:") + chrNewline +
				chrBullet + chrSpace + RomFormat(LibGens::Rom::RFMT_BINARY)
				);
		
		return 4;
	}
	
	// If emulation is already running, simply reset it.
	// TODO: Lock m_rom here and in emuFrameDone().
	bool createEmuThread = true;
	if (m_rom)
	{
		// Save the state of the current ROM first.
		// Don't do a full closeRom(), since that causes
		// issues with threading.
		createEmuThread = false;
		delete m_rom;
		m_rom = NULL;
		
		// TODO: Use a parameter in closeRom() to specify this?
		
		// Make sure SRam/EEPRom data is saved.
		// (SaveData() will call the LibGens OSD handler if necessary.)
		// NOTE: Do this before auto-detecting the region code
		// so SRAM save messages appear before new ROM messages.
		gqt4_emuContext->saveData();
		
		// Close audio.
		// TODO: Do we really need to do this?
		m_audio->close();
	}
	
	// Determine the system region code.
	LibGens::SysVersion::RegionCode_t lg_region = GetLgRegionCode(
				gqt4_config->regionCode(), rom->regionCode(),
				gqt4_config->regionCodeOrder());
	
	if (gqt4_config->regionCode() == GensConfig::CONFREGION_AUTODETECT)
	{
		// Print the auto-detected region.
		const QString detect_str = LgRegionCodeStr(lg_region);
		if (!detect_str.isEmpty())
		{
			//: OSD message indicating the auto-detected ROM region.
			const QString auto_str = tr("ROM region detected as %1.", "osd");
			emit osdPrintMsg(1500, auto_str.arg(detect_str));
		}
	}
	
	// Create a new MD emulation context.
	delete gqt4_emuContext;
	gqt4_emuContext = new LibGens::EmuMD(rom, lg_region);
	rom->close();	// TODO: Let EmuMD handle this...
	
	if (!gqt4_emuContext->isRomOpened())
	{
		// Error loading the ROM image in EmuMD.
		// TODO: EmuMD error code constants.
		// TODO: Show an error message.
		fprintf(stderr, "Error: Initialization of gqt4_emuContext failed. (TODO: Error code.)\n");
		delete gqt4_emuContext;
		gqt4_emuContext = NULL;
		delete rom;
		return 5;
	}
	
	// Save the Rom class pointer as m_rom.
	m_rom = rom;
	
	// m_rom isn't deleted, since keeping it around
	// indicates that a game is running.
	// TODO: Use gqt4_emuContext instead?
	
	// Open audio.
	m_audio->open();
	
	// Initialize timing information.
	m_lastTime = LibGens::Timing::GetTimeD();
	m_lastTime_fps = m_lastTime;
	m_frames = 0;
	
	// Initialize controllers.
	gqt4_config->m_ctrlConfig->updateSysPort(&gqt4_emuContext->m_port1, CtrlConfig::PORT_1);
	gqt4_config->m_ctrlConfig->updateSysPort(&gqt4_emuContext->m_port2, CtrlConfig::PORT_2);
	gqt4_config->m_ctrlConfig->clearDirty();
	
	// Start the emulation thread.
	m_paused.data = 0;
	if (createEmuThread)
	{
		gqt4_emuThread = new EmuThread();
		QObject::connect(gqt4_emuThread, SIGNAL(frameDone(bool)),
				 this, SLOT(emuFrameDone(bool)));
		gqt4_emuThread->start();
	}
	
	// Update the Gens title.
	emit stateChanged();
	return 0;
}

}
