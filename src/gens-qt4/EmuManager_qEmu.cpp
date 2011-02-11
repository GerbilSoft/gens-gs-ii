/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * EmuManager_qEmu.cpp: Emulation manager. (Emulation Queue functions.)    *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#include "EmuManager.hpp"
#include "gqt4_main.hpp"

// ZOMG savestate handler.
#include "libgens/Save/GensZomg.hpp"

// LibGens includes.
#include "libgens/MD/EmuMD.hpp"

// LibGens video includes.
#include "libgens/MD/VdpRend.hpp"
#include "libgens/MD/VdpIo.hpp"
#include "libgens/MD/TAB336.h"

// Controller devices.
#include "libgens/IO/IoBase.hpp"
#include "libgens/IO/Io3Button.hpp"
#include "libgens/IO/Io6Button.hpp"
#include "libgens/IO/Io2Button.hpp"
#include "libgens/IO/IoMegaMouse.hpp"
#include "libgens/IO/IoTeamplayer.hpp"
#include "libgens/IO/Io4WPMaster.hpp"
#include "libgens/IO/Io4WPSlave.hpp"

// Audio backend.
#include "Audio/GensPortAudio.hpp"

// Qt includes.
#include <QtCore/QBuffer>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtGui/QApplication>
#include <QtGui/QImage>
#include <QtGui/QImageWriter>


namespace GensQt4
{

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
			case EmuRequest_t::RQT_CTRLCHANGE:
				// Controller Change.
				doCtrlChange(rq.ctrlChange.port, rq.ctrlChange.ctrlType);
				break;
			
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
			
			case EmuRequest_t::RQT_UNKNOWN:
			default:
				// Unknown emulation request.
				break;
		}
	}
}


/** Emulation Request Queue functions. **/


/**
 * doCtrlChange(): Do a controller change.
 * Called from processQEmuRequest().
 * @param port Controller port. (0, 1, 2) [TODO: Teamplayer/4WP]
 * @param type New controller type.
 */
void EmuManager::doCtrlChange(int port, LibGens::IoBase::IoType type)
{
	// TODO: Teamplayer/4WP support.
	// NOTE: Only works if a ROM is loaded.
	// TODO: Replace this with standard Controller Configuration.
	if (!gqt4_emuContext)
		return;
	
	LibGens::IoBase **prevDevPtr = NULL;
	switch (port)
	{
		case 0:		prevDevPtr = &gqt4_emuContext->m_port1; break;
		case 1:		prevDevPtr = &gqt4_emuContext->m_port2; break;
		case 2:		prevDevPtr = &gqt4_emuContext->m_portE; break;
		default:	break;
	}
	
	if (!(*prevDevPtr))
		return;
	
	if ((*prevDevPtr)->devType() == type)
	{
		// There's no point in changing a controller
		// to the same type that it already is...
		return;
	}
	
	LibGens::IoBase *dev = NULL;
	switch (type)
	{
		case LibGens::IoBase::IOT_NONE:
			// No controller.
			dev = new LibGens::IoBase(*prevDevPtr);
			// TODO: Copy settings from existing Port 1 controller.
			break;
		
		case LibGens::IoBase::IOT_3BTN:
			// 3-button controller.
			dev = new LibGens::Io3Button(*prevDevPtr);
			// TODO: Copy settings from existing Port 1 controller.
			break;
		
		case LibGens::IoBase::IOT_6BTN:
			// 6-button controller.
			dev = new LibGens::Io6Button(*prevDevPtr);
			// TODO: Copy settings from existing Port 1 controller.
			break;
		
		case LibGens::IoBase::IOT_2BTN:
			// 2-button controller.
			dev = new LibGens::Io2Button(*prevDevPtr);
			// TODO: Copy settings from existing Port 1 controller.
			break;
		
		case LibGens::IoBase::IOT_MEGA_MOUSE:
			// Sega Mega Mouse.
			dev = new LibGens::IoMegaMouse(*prevDevPtr);
			// TODO: Copy settings from existing Port 1 controller.
			break;
		
		case LibGens::IoBase::IOT_TEAMPLAYER:
			// Sega Teamplayer.
			dev = new LibGens::IoTeamplayer(*prevDevPtr);
			// TODO: Copy settings from existing Port 1 controller.
			break;
		
		case LibGens::IoBase::IOT_4WP_MASTER:
		case LibGens::IoBase::IOT_4WP_SLAVE:
		{
			// EA 4-Way Play. (TODO)
#if 0
			LibGens::Io4WPMaster *master = new LibGens::Io4WPMaster(LibGens::EmuMD::m_port2);
			LibGens::Io4WPSlave *slave = new LibGens::Io4WPSlave(LibGens::EmuMD::m_port1);
			master->setSlaveDevice(slave);
			port2 = master;
			port1 = slave;
			m_vBackend->osd_printf(1500, "Port 1 set to EA 4-WAY PLAY (Slave).");
			m_vBackend->osd_printf(1500, "Port 2 set to EA 4-WAY PLAY (Master).");
#endif
			break;
		}
		
		default:
			break;
	}
	
	if (!dev)
	{
		// Unknown device. TODO
		return;
	}
			
	// Set the device.
	delete *prevDevPtr;
	*prevDevPtr = dev;
	
	// Print a message on the OSD.
	QString osdMsg = tr("Port %1 set to %2.");
	osdMsg = osdMsg.arg(port + 1);	// TODO: Use "E" for Port 3.
	osdMsg = osdMsg.arg(QString::fromLatin1(dev->devName()));
	
	emit osdPrintMsg(1500, osdMsg);
	
	// Make sure there's no dangling 4WP master/slave device.
	// TODO: Do this after the emulation request queue is cleared.
#if 0
	LibGens::IoBase::IoType p1_type = LibGens::EmuMD::m_port1->devType();
	LibGens::IoBase::IoType p2_type = LibGens::EmuMD::m_port2->devType();
	bool p1_is4WP = (p1_type == LibGens::IoBase::IOT_4WP_MASTER || p1_type == LibGens::IoBase::IOT_4WP_SLAVE);
	bool p2_is4WP = (p2_type == LibGens::IoBase::IOT_4WP_MASTER || p2_type == LibGens::IoBase::IOT_4WP_SLAVE);
	
	if (p1_is4WP ^ p2_is4WP)
	{
		// Dangling 4WP device.
		if (p1_is4WP)
		{
			port1 = new LibGens::IoBase(LibGens::EmuMD::m_port1);
			delete LibGens::EmuMD::m_port1;
			LibGens::EmuMD::m_port1 = port1;
			m_vBackend->osd_printf(1500, "Port 1 set to NONE.");
		}
		else
		{
			port2 = new LibGens::IoBase(LibGens::EmuMD::m_port2);
			delete LibGens::EmuMD::m_port2;
			LibGens::EmuMD::m_port2 = port2;
			m_vBackend->osd_printf(1500, "Port 2 set to NONE.");
		}
	}
#endif
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
	const QString scrFilenameSuffix = QString::fromLatin1(".png");
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
	
	// Print a message on the OSD.
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
	
	// Print a message on the OSD.
	QString osdMsg = tr("Audio sampling rate set to %L1 Hz.");
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
	
	// Print a message on the OSD.
	QString osdMsg = tr("Audio set to %1.");
	osdMsg = osdMsg.arg(newStereo ? tr("Stereo") : tr("Mono"));
	emit osdPrintMsg(1500, osdMsg);
}


/**
 * doSaveState(): Save the current emulation state.
 * @param filename Filename.
 * @param saveSlot Save slot number.
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
			osdMsg = tr("State %1 saved.").arg(saveSlot);
		else
			osdMsg = tr("State saved in %1").arg(sFilename);
	}
	else
	{
		// Error loading savestate.
		osdMsg = tr("Error saving state: %1").arg(ret);
	}
	
	// Print a message on the OSD.
	emit osdPrintMsg(1500, osdMsg);
}


/**
 * doLoadState(): Load the emulation state from a file.
 * @param filename Filename.
 * @param saveSlot Save slot number.
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
			osdMsg = tr("State %1 loaded.").arg(saveSlot);
		else
			osdMsg = tr("State loaded from %1").arg(sFilename);
	}
	else
	{
		// Error loading savestate.
		osdMsg = tr("Error loading state: %1").arg(ret);
	}
	
	// Print a message on the OSD.
	emit osdPrintMsg(1500, osdMsg);
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
		emit osdPrintMsg(2500, tr("Hard Reset."));
	}
	else
	{
		// Do a soft reset.
		gqt4_emuContext->softReset();
		emit osdPrintMsg(2500, tr("Soft Reset."));
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
		
		default:
			break;
	}
}

}
